"""pywinauto automation for the Brimstone MFC GUI.

Launches Brimstone, imports a DICOM series, runs Plan Setup (beam creation +
beamlet dose-calc), sets per-structure prescriptions, and starts the optimizer.

Requires: pip install pywinauto
Uses the "win32" backend, since Brimstone is a classic MFC/Win32 app (dialog
controls are matched by their resource control_id, which is far more reliable
than matching by title/position for this kind of UI).

Edit the CONFIG section below before running. This script is inherently a bit
fragile -- it drives the actual window controls, so if a dialog's resource
layout changes (control IDs, menu text, dialog captions), the corresponding
step here needs updating too. See the control IDs cross-referenced against
Brimstone/Resource.h in the comments below.

Usage:
    python automate_brimstone_ui.py
"""

import os
import time

from pywinauto import Application, win32defines
from pywinauto.keyboard import send_keys

# ------------------------------------------------------------------
# CONFIG -- edit before running
# ------------------------------------------------------------------

EXE_PATH = r"E:\github_dg1an3\dH\x64\Release\Brimstone.exe"

# Folder containing the DICOM series to import (CT slices + RTSTRUCT).
# The importer (CSeriesDicomImporter) is a plain multi-file Open dialog --
# this script navigates into the folder and selects every file in it.
# (pulled from the Import DICOM dialog's own last-visited-folder MRU:
# HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ComDlg32\LastVisitedPidlMRU,
# the "Brimstone.exe" entry -- resolved via SHGetPathFromIDListW)
DICOM_FOLDER = r"C:\Users\Derek\Downloads\hnum_dicom_micro\dicom_micro"

# Number of BEAMS (not beamlets -- the per-beam beamlet count is hardcoded at 39,
# from nBeamletCount=19 in PlanSetupDlg.cpp:71 and again in PlanPyramid.cpp:113).
# BRIMSTONE_BEAM_COUNT overrides, so a sweep driver can vary it per run the same
# way it varies BRIMSTONE_ENTROPY_WEIGHT.
BEAM_COUNT = int(os.environ.get("BRIMSTONE_BEAM_COUNT", 5))
ISOCENTER_XYZ = (218.0, 218.0, -172.0)  # mm offsets -> IDC_EDIT_ISO_OFS_X/Y/Z (default for hnum_dicom_micro)

# One entry per structure to prescribe. "type" must be "Target" or "OAR"
# (matches the IDC_STRUCTTYPE combo text exactly). Selecting a type auto-creates
# a KLDivTerm with a default dose interval (Target: 60-70 Gy, OAR: 0-30 Gy) --
# set dose_min/dose_max to None to keep that default, or give explicit Gy
# values to override it via the "Edit Interval" checkbox + dose edit boxes.
#
# "priority" is the "Precedent" field. Structure::CalcRegion subtracts the region
# of every LOWER-priority structure from this one, so a structure at priority 2
# becomes "itself minus everything at the default priority 1" -- which is how you
# express a background/normal-tissue volume like External. Omit to leave the
# default of 1 (Structure.cpp:20).
STRUCTURE_PRESCRIPTIONS = [
    {"name": "PTV", "type": "Target", "dose_min": 60.0, "dose_max": 70.0, "weight": 2.5},
    {"name": "Spinal Cord", "type": "OAR", "dose_min": 0.0, "dose_max": 30.0, "weight": 2.5},
    {"name": "Parotid(L)", "type": "OAR", "dose_min": 0.0, "dose_max": 30.0, "weight": 0.15},
    {"name": "Parotid(R)", "type": "OAR", "dose_min": 0.0, "dose_max": 30.0, "weight": 0.15},
    {"name": "External", "type": "OAR", "dose_min": 10.0, "dose_max": 50.0, "weight": 0.15,
     "priority": 2},
]

# timeouts, in seconds
LAUNCH_TIMEOUT = 20
IMPORT_TIMEOUT = 120  # DICOM import runs synchronously on the UI thread
BEAMLET_CALC_TIMEOUT = 600  # background dose-calc thread; scales with beam/beamlet count
BEAMLET_POLL_INTERVAL = 2

# ------------------------------------------------------------------
# control IDs, cross-referenced against Brimstone/Resource.h
# ------------------------------------------------------------------

IDC_EDIT_BEAMCOUNT = 1008
IDC_EDIT_ISO_OFS_X = 1009
IDC_EDIT_ISO_OFS_Y = 1010
IDC_EDIT_ISO_OFS_Z = 1011
IDC_ATBEAM = 1019
ID_GO = 1021
IDCANCEL = 2  # "Done" button on the Plan Setup dialog

IDC_STRUCTSELECT = 1005
IDC_STRUCTWEIGHT = 1007
IDC_DOSE1_EDIT2 = 1012
IDC_DOSE2_EDIT2 = 1013
IDC_PRIO_EDIT = 1023
IDC_BTN_SETINTERVAL = 1024
IDC_STRUCTTYPE = 1028
IDC_BTNOPTIMIZE = 1029


def launch_app():
    app = Application(backend="win32").start(EXE_PATH)
    main_win = app.window(title_re=".* - Brimstone$")
    main_win.wait("exists ready", timeout=LAUNCH_TIMEOUT)
    return app, main_win


def import_dicom(app, main_win, folder):
    # CBrimstoneDoc::OnFileImportDcm -- File -> Import DICOM...
    main_win.set_focus()
    main_win.menu_select("File->Import DICOM...")

    open_dlg = app.window(class_name="#32770")  # standard Explorer-style Open dialog
    open_dlg.wait("ready", timeout=10)

    # navigate into the target folder via the filename field
    edit = open_dlg.child_window(class_name="Edit", found_index=0)
    edit.set_edit_text(str(folder))
    send_keys("{ENTER}")
    time.sleep(1)

    # NOTE: the filename field has a hard EM_LIMITTEXT of 259 chars (classic
    # MAX_PATH - 1), so typing every filename directly (as this script used
    # to do) silently truncates mid-name for anything but a couple of short
    # filenames -- these DICOM filenames alone (without even the folder path)
    # already exceed that budget for a 6-file series. Selecting all files in
    # the list view has no such limit, but blind Ctrl+A only works if the list
    # view (not the filename edit box we just typed into) actually has focus,
    # so click into the file-listing pane first -- an offset well to the
    # right of the navigation tree sidebar, comfortably inside the view
    # regardless of window size -- before selecting.
    file_view = open_dlg.child_window(class_name="DirectUIHWND", found_index=0)
    file_view.click_input(coords=(600, 300))
    send_keys("^a")
    open_dlg.child_window(title="&Open", class_name="Button").click()

    # import (CSeriesDicomImporter::ProcessNext loop) runs synchronously on the
    # UI thread with no progress dialog, so wait for the app to accept input
    # again rather than for a window to close
    main_win.wait("ready", timeout=IMPORT_TIMEOUT)


def run_plan_setup(app, main_win, beam_count, isocenter):
    # Plan -> Plan Setup (ID_GENBEAMLETS) -> CPlanSetupDlg
    main_win.menu_select("Plan->Plan Setup")

    dlg = app.window(title="Plan Setup")
    dlg.wait("ready", timeout=10)

    dlg.child_window(control_id=IDC_EDIT_BEAMCOUNT, class_name="Edit").set_edit_text(str(beam_count))
    dlg.child_window(control_id=IDC_EDIT_ISO_OFS_X, class_name="Edit").set_edit_text(str(isocenter[0]))
    dlg.child_window(control_id=IDC_EDIT_ISO_OFS_Y, class_name="Edit").set_edit_text(str(isocenter[1]))
    dlg.child_window(control_id=IDC_EDIT_ISO_OFS_Z, class_name="Edit").set_edit_text(str(isocenter[2]))

    dlg.child_window(control_id=ID_GO).click()

    # CPlanSetupDlg::OnBnClickedGo starts a background thread (CalculateBeamlets)
    # that beam-by-beam ray-traces/convolves beamlets; OnDoseCalcThreadDone sets
    # this field's text to "Done" when the whole plan finishes
    at_beam = dlg.child_window(control_id=IDC_ATBEAM, class_name="Edit")
    deadline = time.time() + BEAMLET_CALC_TIMEOUT
    while time.time() < deadline:
        if at_beam.window_text().strip() == "Done":
            break
        time.sleep(BEAMLET_POLL_INTERVAL)
    else:
        raise TimeoutError("Beamlet generation did not finish within BEAMLET_CALC_TIMEOUT")

    dlg.child_window(control_id=IDCANCEL).click()  # "Done" button, closes the dialog
    main_win.wait("ready", timeout=10)


def set_structure_prescription(main_win, struct_cfg):
    # CPrescriptionToolbar is a CDialogBar docked in the main frame -- its
    # controls are reachable as descendants of the main window
    struct_select = main_win.child_window(control_id=IDC_STRUCTSELECT, class_name="ComboBox")
    # this pywinauto version's ComboBoxWrapper has no expand()/collapse(), so fire
    # the CBN_DROPDOWN notification directly -- OnDropdownStructselectcombo()
    # populates the list only in response to that notification
    struct_select.notify_parent(win32defines.CBN_DROPDOWN)
    time.sleep(0.2)
    struct_select.select(struct_cfg["name"])
    time.sleep(0.3)

    struct_type = main_win.child_window(control_id=IDC_STRUCTTYPE, class_name="ComboBox")
    struct_type.select(struct_cfg["type"])  # auto-creates the KLDivTerm w/ default interval
    time.sleep(0.3)

    # "Precedent" -- set right after the type selection, so the region exclusion is
    # in effect before the interval binning and the histogram regions get built.
    # OnEnChangePrioEdit fires on EN_CHANGE and needs the KLDivTerm to already
    # exist, which the type selection above guarantees.
    priority = struct_cfg.get("priority")
    if priority is not None:
        prio_edit = main_win.child_window(control_id=IDC_PRIO_EDIT, class_name="Edit")
        prio_edit.set_edit_text(str(int(priority)))
        send_keys("{TAB}")
        time.sleep(0.3)

    dose_min = struct_cfg.get("dose_min")
    dose_max = struct_cfg.get("dose_max")
    if dose_min is not None and dose_max is not None:
        set_interval_btn = main_win.child_window(control_id=IDC_BTN_SETINTERVAL, class_name="Button")
        dose1_edit = main_win.child_window(control_id=IDC_DOSE1_EDIT2, class_name="Edit")
        dose2_edit = main_win.child_window(control_id=IDC_DOSE2_EDIT2, class_name="Edit")

        set_interval_btn.check()  # OnBnClickedBtnSetinterval: enables the dose edit boxes
        # The dose edits hold Gy as a plain integer: OnBnClickedBtnSetinterval reads
        # the box and divides by 100 to get the internal value (Gy/100), which is why
        # the display path at PrescriptionToolbar.cpp:150 multiplies GetMinDose() by
        # 100 to get back to Gy. That *100 is the round-trip, NOT an extra scaling --
        # writing Gy*100 here prescribes 100x the intended dose.
        dose1_edit.set_edit_text(str(int(round(dose_min))))
        dose2_edit.set_edit_text(str(int(round(dose_max))))
        set_interval_btn.uncheck()  # commits SetInterval() on un-check

    weight = struct_cfg.get("weight")
    if weight is not None:
        weight_edit = main_win.child_window(control_id=IDC_STRUCTWEIGHT, class_name="Edit")
        weight_edit.set_edit_text(f"{weight:.2f}")
        send_keys("{TAB}")


def start_optimizer(main_win):
    # IDC_BTNOPTIMIZE toggles CBrimstoneView::OnOptimize -- click once to start
    main_win.child_window(control_id=IDC_BTNOPTIMIZE, class_name="Button").click()


def main():
    app, main_win = launch_app()
    import_dicom(app, main_win, DICOM_FOLDER)
    run_plan_setup(app, main_win, BEAM_COUNT, ISOCENTER_XYZ)
    for struct_cfg in STRUCTURE_PRESCRIPTIONS:
        set_structure_prescription(main_win, struct_cfg)
    start_optimizer(main_win)


if __name__ == "__main__":
    main()

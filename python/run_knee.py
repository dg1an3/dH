r"""Launch Brimstone detached with the current BRIMSTONE_* env, drive the
standard 7-beam plan setup via automate_brimstone_ui, start the optimizer, print
the converged objective, and leave the window open.

Invoked by run_brimstone_knee_7beam.bat, which sets the entropy/knee env vars.
The detached child inherits this process's environment (hence the .bat's vars),
and survives this script's exit (DETACHED_PROCESS), so the window stays up.
"""
import os
import sys
import time
import subprocess

REPO = r"C:\dev_dH_and_friends\dH"
EXE = os.path.join(REPO, r"x64\Release\Brimstone.exe")
sys.path.insert(0, os.path.join(REPO, "python"))

DETACHED_PROCESS = 0x00000008
CREATE_NEW_PROCESS_GROUP = 0x00000200


def main():
    proc = subprocess.Popen(
        [EXE],
        cwd=os.path.join(REPO, "Brimstone"),
        creationflags=DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP,
        close_fds=True,
    )
    print("launched Brimstone pid=%d (separable, W=%s, %s beams)" % (
        proc.pid,
        os.environ.get("BRIMSTONE_ENTROPY_WEIGHT", "?"),
        os.environ.get("BRIMSTONE_BEAM_COUNT", "7"),
    ), flush=True)

    import automate_brimstone_ui as drv
    drv.EXE_PATH = EXE  # unused in attach mode
    drv.BEAM_COUNT = int(os.environ.get("BRIMSTONE_BEAM_COUNT", "7"))
    os.environ["BRIMSTONE_ATTACH_PID"] = str(proc.pid)

    drv.main()  # import DICOM -> plan setup -> prescriptions -> start optimizer
    print("optimizer started; window will stay open", flush=True)

    obj = os.environ.get("BRIMSTONE_OBJECTIVE_FILE")
    if obj:
        deadline = time.time() + 900
        while time.time() < deadline:
            if os.path.exists(obj) and os.path.getsize(obj) > 0:
                time.sleep(0.3)
                print("CONVERGED:", open(obj).read().strip(), flush=True)
                break
            if proc.poll() is not None:
                print("Brimstone exited before writing a result", flush=True)
                break
            time.sleep(2)


if __name__ == "__main__":
    main()

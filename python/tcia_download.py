#!/usr/bin/env python
"""Download CT + RTSTRUCT DICOM series from a TCIA collection via the public
NBIA REST API, laid out per patient for import into Brimstone.

Default target is NSCLC-Radiomics: a fully public (no application) TCIA
collection (CC BY-NC 3.0) with CT + RTSTRUCT delineations of the gross tumor
volume plus lung / heart / esophagus. It is a practical stand-in for the MD
Anderson HNSCC collection, whose imaging is behind NIH Controlled Data Access
(the head scans could reconstruct a face).

The same script can pull a controlled-access collection (e.g. HNSCC) once you
have been granted access -- pass --user/--password and the script will obtain
an OAuth token and send it with every request.

Output layout:
    <out>/<PatientID>/<Modality>_<SeriesInstanceUID>/*.dcm

Usage:
    python tcia_download.py                         # 3 NSCLC-Radiomics patients
    python tcia_download.py --patients 5 --out data
    python tcia_download.py --collection HNSCC --user me --password ***  # controlled

Requires: requests  (pip install requests)

Copyright (c) 2007-2021, Derek G. Lane. See LICENSE.
"""
import argparse
import io
import os
import sys
import time
import zipfile

try:
    import requests
except ImportError:
    sys.exit("This script needs the 'requests' package:  pip install requests")

API = "https://services.cancerimagingarchive.net/nbia-api/services/v1"
TOKEN_URL = "https://services.cancerimagingarchive.net/nbia-api/oauth/token"


def get_token(user, password):
    """Obtain an OAuth access token for controlled-access collections."""
    resp = requests.post(
        TOKEN_URL,
        data={
            "username": user,
            "password": password,
            "grant_type": "password",
            "client_id": "nbiaRestAPIClient",
            "client_secret": "ItsBetweenUAndMe",  # documented public client secret
        },
        timeout=60,
    )
    resp.raise_for_status()
    return resp.json()["access_token"]


def api_get(path, headers, **params):
    """GET a JSON endpoint, returning the parsed body (list/dict)."""
    resp = requests.get(f"{API}/{path}", params=params, headers=headers, timeout=120)
    resp.raise_for_status()
    if not resp.content:
        return []
    return resp.json()


def download_series(series_uid, dest_dir, headers):
    """Fetch one series (a DICOM zip) and extract it into dest_dir."""
    resp = requests.get(
        f"{API}/getImage",
        params={"SeriesInstanceUID": series_uid},
        headers=headers,
        timeout=600,
        stream=True,
    )
    resp.raise_for_status()
    with zipfile.ZipFile(io.BytesIO(resp.content)) as zf:
        zf.extractall(dest_dir)
    return len(os.listdir(dest_dir))


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--collection", default="NSCLC-Radiomics",
                    help="TCIA collection name (default: NSCLC-Radiomics)")
    ap.add_argument("--patients", type=int, default=3,
                    help="number of patients to download (default: 3)")
    ap.add_argument("--modalities", default="CT,RTSTRUCT",
                    help="comma-separated modalities to keep (default: CT,RTSTRUCT)")
    ap.add_argument("--out", default="tcia_data", help="output directory")
    ap.add_argument("--user", help="TCIA username (only for controlled-access collections)")
    ap.add_argument("--password", help="TCIA password (only for controlled-access collections)")
    args = ap.parse_args()

    wanted = [m.strip().upper() for m in args.modalities.split(",") if m.strip()]

    headers = {}
    if args.user and args.password:
        print("Authenticating for controlled-access download...")
        headers["Authorization"] = "Bearer " + get_token(args.user, args.password)

    print(f"Querying series in collection '{args.collection}'...")
    series = api_get("getSeries", headers, Collection=args.collection)
    if not series:
        sys.exit(f"No series returned for '{args.collection}'. Check the name "
                 f"(case-sensitive) or, for controlled data, pass --user/--password.")

    # group series by patient, keeping only the modalities we care about
    by_patient = {}
    for s in series:
        if s.get("Modality", "").upper() not in wanted:
            continue
        by_patient.setdefault(s["PatientID"], []).append(s)

    # prefer patients that have every requested modality (e.g. CT *and* RTSTRUCT)
    def coverage(pid):
        return len({s["Modality"].upper() for s in by_patient[pid]})

    ordered = sorted(by_patient, key=lambda p: (-coverage(p), p))
    chosen = ordered[: args.patients]
    if not chosen:
        sys.exit("No patients matched the requested modalities.")

    print(f"Found {len(by_patient)} patients with {wanted}; "
          f"downloading {len(chosen)}: {', '.join(chosen)}\n")

    total_series = 0
    for pid in chosen:
        mods = sorted({s["Modality"].upper() for s in by_patient[pid]})
        print(f"[{pid}]  modalities present: {', '.join(mods)}")
        for s in by_patient[pid]:
            uid = s["SeriesInstanceUID"]
            mod = s["Modality"].upper()
            dest = os.path.join(args.out, pid, f"{mod}_{uid}")
            if os.path.isdir(dest) and os.listdir(dest):
                print(f"    - {mod:8s} {uid[:24]}...  (already present, skipped)")
                continue
            os.makedirs(dest, exist_ok=True)
            for attempt in range(3):
                try:
                    n = download_series(uid, dest, headers)
                    print(f"    - {mod:8s} {uid[:24]}...  {n} files")
                    total_series += 1
                    break
                except Exception as exc:  # noqa: BLE001 - report and retry
                    print(f"    ! {mod:8s} attempt {attempt + 1} failed: {exc}")
                    time.sleep(2 * (attempt + 1))
            else:
                print(f"    ! {mod:8s} {uid[:24]}...  GIVING UP")

    print(f"\nDone. {total_series} series into '{os.path.abspath(args.out)}'.")
    print("Cite: NSCLC-Radiomics DOI 10.7937/K9/TCIA.2015.PF0M9REI (CC BY-NC 3.0) "
          "when using this data.")


if __name__ == "__main__":
    main()

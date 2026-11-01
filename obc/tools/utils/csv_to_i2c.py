import pandas as pd
import numpy as np

# ---------- Configurable options ----------
CSV_PATH = "./example_csv/digital.csv"
SCL_COL  = 'Channel 0'
SDA_COL  = 'Channel 1'
TS       = None   # sample period in seconds (1 / sample rate); None = estimate from data

# Min limits from datasheet table (all in seconds)
LIMITS = {
    "tcycle":  2.5e-6,
    "tHSTART": 0.6e-6,
    "tHD":     0.0,      # SDA_OUT case; use 300e-9 for the SDA_IN case
    "tSD":     100e-9,
    "tSSTART": 0.6e-6,
    "tSSTOP":  0.6e-6,
    "tIDLE":   1.3e-6,
}

# ---------- Load ----------
df = pd.read_csv(CSV_PATH)
i2cDf = df[['Time [s]', SCL_COL, SDA_COL]].rename(columns={'Time [s]': 't', SCL_COL: 'SCL', SDA_COL: 'SDA'})

t   = i2cDf['t'].to_numpy(dtype=float)
scl = i2cDf['SCL'].to_numpy().astype(int)
sda = i2cDf['SDA'].to_numpy().astype(int)

# Row 0 is the initial state at capture start, not an edge.
if TS is None:
    TS = np.diff(t).min()   # upper bound on the true sample period
    print(f"TS estimated from smallest row gap: {TS*1e9:.0f} ns (set TS manually if you know it)")

# ---------- Edge detection ----------
dscl, dsda = np.diff(scl), np.diff(sda)
te = t[1:]                              # edge time = row where the new value appears
scl_prev, scl_new = scl[:-1], scl[1:]
sda_chg = dsda != 0

scl_r = te[dscl ==  1]
scl_f = te[dscl == -1]

# SDA edges classified by the SCL level before/after the SDA edge
start = te[(dsda == -1) & (scl_prev == 1) & (scl_new == 1)]   # includes repeated STARTs
stop  = te[(dsda ==  1) & (scl_prev == 1) & (scl_new == 1)]
data  = te[sda_chg & (scl_new == 0)]                          # SDA changes while SCL low,
                                                              # incl. same row as SCL fall (tHD ~ 0)
at_scl_rise = te[sda_chg & (scl_prev == 0) & (scl_new == 1)]  # SDA changes with SCL rising (tSD ~ 0)

# ---------- Helpers ----------
def next_after(a, b):
    """Interval from each a to the first b strictly after it."""
    i = np.searchsorted(b, a, side='right')
    out = np.full(len(a), np.nan); ok = i < len(b)
    out[ok] = b[i[ok]] - a[ok]; return out

def prev_before(a, b, inclusive=False):
    """Interval since the last b before each a (<= if inclusive, else <)."""
    i = np.searchsorted(b, a, side='right' if inclusive else 'left') - 1
    out = np.full(len(a), np.nan); ok = i >= 0
    out[ok] = a[ok] - b[i[ok]]; return out

# Repeated START = START with no STOP since the previous START
is_rep = np.array([i > 0 and not ((stop > start[i-1]) & (stop < start[i])).any()
                   for i in range(len(start))], dtype=bool)

# ---------- Measurements ----------
results = {
    "tcycle":  np.diff(scl_r),
    "tHSTART": next_after(start, scl_f),
    "tHD":     prev_before(data, scl_f, inclusive=True),   # inclusive: 0 ns is legal here
    "tSD":     next_after(data, scl_r),
    "tSSTART": prev_before(start[is_rep], scl_r) if is_rep.any() else np.array([]),
    "tSSTOP":  prev_before(stop, scl_r),
    "tIDLE":   next_after(stop, start),
}

# ---------- Report ----------
rows = []
for name, x in results.items():
    x = x[~np.isnan(x)]
    rows.append({
        "param":       name,
        "n":           len(x),
        "min [ns]":    x.min() * 1e9 if len(x) else np.nan,
        "median [ns]": np.median(x) * 1e9 if len(x) else np.nan,
        "max [ns]":    x.max() * 1e9 if len(x) else np.nan,
        "limit [ns]":  LIMITS[name] * 1e9,
    })

resultsDf = pd.DataFrame(rows).set_index("param")

print(f"\nSDA changes in same row as SCL rising (tSD <= {TS*1e9:.0f} ns): {len(at_scl_rise)}")
print(resultsDf.to_string(float_format=lambda v: f"{v:.0f}"))
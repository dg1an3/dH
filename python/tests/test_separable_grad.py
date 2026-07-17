"""Validate the separable per-beamlet binary-entropy gradient formula against
central finite differences (in isolation, no adaptive-variance confound):
    q_i     = sigmoid(s*x_i)
    H       = sum_i [ -q_i ln q_i - (1-q_i) ln(1-q_i) ]
    dH/dx_i = ln((1-q_i)/q_i) * s*q_i*(1-q_i)     (= ln((1-q)/q)*dSigmoid)
"""
import numpy as np
rng = np.random.default_rng(0)
s = 0.5   # m_inputScale

def sig(x): return 1.0/(1.0+np.exp(-s*x))
def H(x):
    q = np.clip(sig(x), 1e-12, 1-1e-12)
    return np.sum(-(q*np.log(q) + (1-q)*np.log(1-q)))
def gA(x):
    q = np.clip(sig(x), 1e-12, 1-1e-12)
    return np.log((1-q)/q) * (s*q*(1-q))
def gN(x, eps=1e-6):
    g=np.zeros_like(x)
    for k in range(len(x)):
        xp=x.copy(); xp[k]+=eps; xm=x.copy(); xm[k]-=eps
        g[k]=(H(xp)-H(xm))/(2*eps)
    return g

worst=0.0
for _ in range(50):
    n=int(rng.integers(3,200)); sc=float(rng.choice([0.1,1.0,5.0,10.0]))
    x=rng.normal(0,sc,size=n)
    a,nu=gA(x),gN(x)
    nw=np.linalg.norm(a-nu)/(np.linalg.norm(a)+np.linalg.norm(nu)+1e-30)
    worst=max(worst,nw)
print(f"worst norm-wise rel error: {worst:.3e}")
print("PASS" if worst<1e-6 else "FAIL")

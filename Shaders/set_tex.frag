#if NTEXS >0
    TEX_VARS
#endif
#if NBUMPS >0
    BUMP_VARS
#endif
#if NVALS >0
    NOISE_VARS
#endif

#ifdef TX0
#ifdef A0
    SET_ATTRIB(A0)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(0,C0)
#ifdef N0
    SET_NOISE(N0)
#endif
#ifdef X0
    SET_TEX(X0)
#endif
    APPLY_TEX
#ifdef M0
    BGN_ORDERS
#endif
#ifdef T0
    SET_COLOR
#endif
#ifdef B0
    SET_BUMP
#endif
#ifdef M0
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX1
#ifdef A1
    SET_ATTRIB(A1)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(1,C1)
#ifdef N1
    SET_NOISE(N1)
#endif
#ifdef X1
    SET_TEX(X1)
#endif
    APPLY_TEX
#ifdef M1
    BGN_ORDERS
#endif
#ifdef T1
    SET_COLOR
#endif
#ifdef B1
    SET_BUMP
#endif
#ifdef M1
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX2
#ifdef A2
    SET_ATTRIB(A2)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(2,C2)
#ifdef N2
    SET_NOISE(N2)
#endif
#ifdef X2
    SET_TEX(X2)
#endif
    APPLY_TEX
#ifdef M2
    BGN_ORDERS
#endif
#ifdef T2
    SET_COLOR
#endif
#ifdef B2
    SET_BUMP
#endif
#ifdef M2
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX3
#ifdef A3
    SET_ATTRIB(A3)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(3,C3)
#ifdef N3
    SET_NOISE(N3)
#endif
#ifdef X3
    SET_TEX(X3)
#endif
    APPLY_TEX
#ifdef M3
    BGN_ORDERS
#endif
#ifdef T3
    SET_COLOR
#endif
#ifdef B3
    SET_BUMP
#endif
#ifdef M3
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX4
#ifdef A4
    SET_ATTRIB(A4)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(4,C4)
#ifdef N4
    SET_NOISE(N4)
#endif
#ifdef X4
    SET_TEX(X4)
#endif
    APPLY_TEX
#ifdef M4
    BGN_ORDERS
#endif
#ifdef T4
    SET_COLOR
#endif
#ifdef B4
    SET_BUMP
#endif
#ifdef M4
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX5
#ifdef A5
    SET_ATTRIB(A5)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(5,C5)
#ifdef N5
    SET_NOISE(N5)
#endif
#ifdef X5
    SET_TEX(X5)
#endif
    APPLY_TEX
#ifdef M5
    BGN_ORDERS
#endif
#ifdef T5
    SET_COLOR
#endif
#ifdef B5
    SET_BUMP
#endif
#ifdef M5
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX6
#ifdef A6
    SET_ATTRIB(A6)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(6,C6)
#ifdef N6
    SET_NOISE(N6)
#endif
#ifdef X6
    SET_TEX(X6)
#endif
    APPLY_TEX
#ifdef M6
    BGN_ORDERS
#endif
#ifdef T6
    SET_COLOR
#endif
#ifdef B6
    SET_BUMP
#endif
#ifdef M6
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX7
#ifdef A7
    SET_ATTRIB(A7)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(7,C7)
#ifdef N7
    SET_NOISE(N7)
#endif
#ifdef X7
    SET_TEX(X7)
#endif
    APPLY_TEX
#ifdef M7
    BGN_ORDERS
#endif
#ifdef T7
    SET_COLOR
#endif
#ifdef B7
    SET_BUMP
#endif
#ifdef M7
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX8
INIT_TEX(8,C8)
#ifdef N8
    SET_NOISE(N8)
#endif
#ifdef X8
    SET_TEX(X8)
#endif
    APPLY_TEX
#ifdef M8
    BGN_ORDERS
#endif
#ifdef T8
    SET_COLOR
#endif
#ifdef B8
    SET_BUMP
#endif
#ifdef M8
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX9
INIT_TEX(9,C9)
#ifdef N9
    SET_NOISE(N9)
#endif
#ifdef X9
    SET_TEX(X9)
#endif
    APPLY_TEX
#ifdef M9
    BGN_ORDERS
#endif
#ifdef T9
    SET_COLOR
#endif
#ifdef B9
    SET_BUMP
#endif
#ifdef M9
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX10
INIT_TEX(10,C10)
#ifdef N10
    SET_NOISE(N10)
#endif
#ifdef X10
    SET_TEX(X10)
#endif
    APPLY_TEX
#ifdef M10
    BGN_ORDERS
#endif
#ifdef T10
    SET_COLOR
#endif
#ifdef B10
    SET_BUMP
#endif
#ifdef M10
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX11
INIT_TEX(11,C11)
#ifdef N11
    SET_NOISE(N11)
#endif
#ifdef X11
    SET_TEX(X11)
#endif
    APPLY_TEX
#ifdef M11
    BGN_ORDERS
#endif
#ifdef T11
    SET_COLOR
#endif
#ifdef B11
    SET_BUMP
#endif
#ifdef M11
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX12
INIT_TEX(12,C12)
#ifdef N12
    SET_NOISE(N12)
#endif
#ifdef X12
    SET_TEX(X12)
#endif
    APPLY_TEX
#ifdef M12
    BGN_ORDERS
#endif
#ifdef T12
    SET_COLOR
#endif
#ifdef B12
    SET_BUMP
#endif
#ifdef M12
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX13
INIT_TEX(13,C13)
#ifdef N13
    SET_NOISE(N13)
#endif
#ifdef X13
    SET_TEX(X13)
#endif
    APPLY_TEX
#ifdef M13
    BGN_ORDERS
#endif
#ifdef T13
    SET_COLOR
#endif
#ifdef B13
    SET_BUMP
#endif
#ifdef M13
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX14
INIT_TEX(14,C14)
#ifdef N14
    SET_NOISE(N14)
#endif
#ifdef X14
    SET_TEX(X14)
#endif
    APPLY_TEX
#ifdef M14
    BGN_ORDERS
#endif
#ifdef T14
    SET_COLOR
#endif
#ifdef B14
    SET_BUMP
#endif
#ifdef M14
    NEXT_ORDER
    END_ORDERS
#endif
#endif

#ifdef TX15
INIT_TEX(15,C15)
#ifdef N15
    SET_NOISE(N15)
#endif
#ifdef X15
    SET_TEX(X15)
#endif
    APPLY_TEX
#ifdef M15
    BGN_ORDERS
#endif
#ifdef T15
    SET_COLOR
#endif
#ifdef B15
    SET_BUMP
#endif
#ifdef M15
    NEXT_ORDER
    END_ORDERS
#endif
#endif


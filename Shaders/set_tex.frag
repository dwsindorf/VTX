#if NVALS >0
    NOISE_VARS
#endif

#ifdef TX0
if(tex2d[0].instance == tex2d[0].active) {
#ifdef A0
    SET_ATTRIB(A0)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(0,C0)
#ifdef N0
    SET_NOISE(N0)
#endif
#ifdef M0
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX1
if(tex2d[1].instance == tex2d[1].active) {
#ifdef A1
    SET_ATTRIB(A1)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(1,C1)
#ifdef N1
    SET_NOISE(N1)
#endif
#ifdef M1
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX2
if(tex2d[2].instance == tex2d[2].active) {
#ifdef A2
    SET_ATTRIB(A2)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(2,C2)
#ifdef N2
    SET_NOISE(N2)
#endif
#ifdef M2
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX3
if(tex2d[3].instance == tex2d[3].active) {
#ifdef A3
    SET_ATTRIB(A3)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(3,C3)
#ifdef N3
    SET_NOISE(N3)
#endif
#ifdef M3
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX4
if(tex2d[4].instance == tex2d[4].active) {
#ifdef A4
    SET_ATTRIB(A4)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(4,C4)
#ifdef N4
    SET_NOISE(N4)
#endif
#ifdef M4
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX5
if(tex2d[5].instance == tex2d[5].active) {
#ifdef A5
    SET_ATTRIB(A5)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(5,C5)
#ifdef N5
    SET_NOISE(N5)
#endif
#ifdef M5
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX6
if(tex2d[6].instance == tex2d[6].active) {
#ifdef A6
    SET_ATTRIB(A6)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(6,C6)
#ifdef N6
    SET_NOISE(N6)
#endif
#ifdef M6
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX7
if(tex2d[7].instance == tex2d[7].active) {
#ifdef A7
    SET_ATTRIB(A7)
#else
    SET_ATTRIB(1.0)
#endif
INIT_TEX(7,C7)
#ifdef N7
    SET_NOISE(N7)
#endif
#ifdef M7
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX8
if(tex2d[8].instance == tex2d[8].active) {
INIT_TEX(8,C8)
#ifdef N8
    SET_NOISE(N8)
#endif
#ifdef M8
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX9
if(tex2d[9].instance == tex2d[9].active) {
INIT_TEX(9,C9)
#ifdef N9
    SET_NOISE(N9)
#endif
#ifdef M9
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX10
if(tex2d[10].instance == tex2d[10].active) {
INIT_TEX(10,C10)
#ifdef N10
    SET_NOISE(N10)
#endif
#ifdef M10
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX11
if(tex2d[11].instance == tex2d[11].active) {
INIT_TEX(11,C11)
#ifdef N11
    SET_NOISE(N11)
#endif
#ifdef M11
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX12
if(tex2d[12].instance == tex2d[12].active) {
INIT_TEX(12,C12)
#ifdef N12
    SET_NOISE(N12)
#endif
#ifdef M12
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX13
if(tex2d[13].instance == tex2d[13].active) {
INIT_TEX(13,C13)
#ifdef N13
    SET_NOISE(N13)
#endif
#ifdef M13
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX14
if(tex2d[14].instance == tex2d[14].active) {
INIT_TEX(14,C14)
#ifdef N14
    SET_NOISE(N14)
#endif
#ifdef M14
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif

#ifdef TX15
if(tex2d[15].instance == tex2d[15].active) {
INIT_TEX(15,C15)
#ifdef N15
    SET_NOISE(N15)
#endif
#ifdef M15
    BGN_ORDERS
#endif
    APPLY_TEX
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
}
#endif


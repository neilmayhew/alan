
act.o: act.c alan.h sysdep.h types.h srcp.h lst.h dump.h adv.h whr.h \
 evt.h nam.h acode.h wht.h sym.h elm.h res.h stx.h exp.h atr.h emit.h \
 stm.h obj.h cnt.h act.h loc.h ext.h scr.h wrd.h vrb.h lmList.h \
 alanCommon.h token.h
adv.o: adv.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h acode.h adv.h whr.h evt.h nam.h wht.h sym.h \
 elm.h res.h stx.h stm.h exp.h obj.h cnt.h act.h loc.h ext.h vrb.h \
 rul.h wrd.h syn.h atr.h emit.h msg.h sco.h opt.h encode.h
alan.o: alan.c sysdep.h types.h alan.h srcp.h lst.h dump.h \
 alan.version.h version.h adv.h whr.h evt.h nam.h acode.h wht.h opt.h \
 lmList.h alanCommon.h token.h pmParse.h smScan.h options.h timing.h
alan.version.o: alan.version.c alan.version.h version.h
alt.o: alt.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h sym.h nam.h acode.h elm.h res.h stx.h alt.h \
 act.h cnt.h wht.h whr.h evt.h chk.h exp.h stm.h obj.h loc.h ext.h \
 emit.h
atr.o: atr.c alan.h sysdep.h types.h srcp.h lst.h dump.h adv.h whr.h \
 evt.h nam.h acode.h wht.h atr.h sym.h elm.h res.h stx.h exp.h emit.h \
 obj.h cnt.h loc.h ext.h act.h opt.h lmList.h alanCommon.h token.h \
 encode.h
chk.o: chk.c alan.h sysdep.h types.h srcp.h lst.h dump.h exp.h wht.h \
 nam.h acode.h whr.h evt.h stm.h obj.h cnt.h act.h loc.h ext.h chk.h \
 emit.h
cnt.o: cnt.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h acode.h adv.h whr.h evt.h nam.h wht.h sym.h \
 elm.h res.h stx.h stm.h exp.h obj.h cnt.h act.h loc.h ext.h lim.h \
 atr.h emit.h
dump.o: dump.c alan.h sysdep.h types.h srcp.h lst.h dump.h adv.h whr.h \
 evt.h nam.h acode.h wht.h atr.h sym.h elm.h res.h stx.h exp.h emit.h \
 stm.h obj.h cnt.h act.h loc.h ext.h chk.h vrb.h lim.h scr.h stp.h \
 rul.h lmList.h alanCommon.h token.h
elm.o: elm.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h sym.h nam.h acode.h elm.h res.h stx.h wrd.h \
 emit.h
emit.o: emit.c sysdep.h types.h alan.h srcp.h lst.h dump.h acode.h \
 alan.version.h version.h emit.h
encode.o: encode.c alan.h sysdep.h types.h srcp.h lst.h dump.h acode.h \
 opt.h emit.h encode.h
evt.o: evt.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h adv.h whr.h evt.h nam.h acode.h wht.h sym.h \
 elm.h res.h stx.h stm.h exp.h obj.h cnt.h act.h loc.h ext.h opt.h \
 emit.h
exp.o: exp.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h adv.h whr.h evt.h nam.h acode.h wht.h sym.h \
 elm.h res.h stx.h exp.h atr.h emit.h obj.h cnt.h loc.h ext.h act.h \
 encode.h
ext.o: ext.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h sym.h nam.h acode.h elm.h res.h stx.h stm.h \
 wht.h whr.h evt.h exp.h obj.h cnt.h act.h loc.h ext.h chk.h wrd.h \
 emit.h
lim.o: lim.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h adv.h whr.h evt.h nam.h acode.h wht.h exp.h \
 atr.h sym.h elm.h res.h stx.h emit.h stm.h obj.h cnt.h act.h loc.h \
 ext.h chk.h vrb.h lim.h rul.h
lmList.o: lmList.c lmList.h alanCommon.h srcp.h token.h
loc.o: loc.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h adv.h whr.h evt.h nam.h acode.h wht.h sym.h \
 elm.h res.h stx.h exp.h atr.h emit.h loc.h ext.h stm.h obj.h cnt.h \
 act.h vrb.h
lst.o: lst.c alan.h sysdep.h types.h srcp.h lst.h dump.h nam.h acode.h \
 act.h cnt.h wht.h whr.h evt.h alt.h atr.h sym.h elm.h res.h stx.h \
 exp.h emit.h chk.h ext.h lim.h loc.h obj.h rul.h scr.h stm.h stp.h \
 syn.h vrb.h
macSpa.o: macSpa.c
main.o: main.c alan.h sysdep.h types.h srcp.h lst.h dump.h spa.h \
 options.h alan.version.h version.h
msg.o: msg.c sysdep.h alan.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h adv.h whr.h evt.h nam.h acode.h wht.h msg.h \
 stm.h exp.h obj.h cnt.h act.h loc.h ext.h opt.h emit.h encode.h
nam.o: nam.c alan.h sysdep.h types.h srcp.h lst.h dump.h nam.h acode.h \
 str.h obj.h whr.h evt.h wht.h cnt.h act.h ext.h loc.h vrb.h stx.h \
 msg.h emit.h
obj.o: obj.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h adv.h whr.h evt.h nam.h acode.h wht.h sym.h \
 elm.h res.h stx.h exp.h atr.h emit.h stm.h obj.h cnt.h act.h loc.h \
 ext.h wrd.h vrb.h
opt.o: opt.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h acode.h opt.h emit.h
options.o: options.c types.h dump.h sysdep.h options.h lst.h
pmErr.o: pmErr.c srcp.h smScan.h alanCommon.h token.h sysdep.h types.h \
 lst.h dump.h lmList.h act.h nam.h alan.h acode.h cnt.h wht.h whr.h \
 evt.h adv.h alt.h atr.h sym.h elm.h res.h stx.h exp.h emit.h chk.h \
 ext.h lim.h loc.h msg.h obj.h opt.h rul.h sco.h scr.h stm.h stp.h \
 str.h syn.h vrb.h
pmPaSema.o: pmPaSema.c srcp.h smScan.h alanCommon.h token.h sysdep.h \
 types.h lst.h dump.h lmList.h act.h nam.h alan.h acode.h cnt.h wht.h \
 whr.h evt.h adv.h alt.h atr.h sym.h elm.h res.h stx.h exp.h emit.h \
 chk.h ext.h lim.h loc.h msg.h obj.h opt.h rul.h sco.h scr.h stm.h \
 stp.h str.h syn.h vrb.h
pmParse.o: pmParse.c srcp.h smScan.h alanCommon.h token.h sysdep.h \
 types.h lst.h dump.h lmList.h act.h nam.h alan.h acode.h cnt.h wht.h \
 whr.h evt.h adv.h alt.h atr.h sym.h elm.h res.h stx.h exp.h emit.h \
 chk.h ext.h lim.h loc.h msg.h obj.h opt.h rul.h sco.h scr.h stm.h \
 stp.h str.h syn.h vrb.h pmParse.h
res.o: res.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h res.h stx.h nam.h acode.h sym.h elm.h stm.h \
 wht.h whr.h evt.h exp.h obj.h cnt.h act.h loc.h ext.h emit.h
rul.o: rul.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h acode.h adv.h whr.h evt.h nam.h wht.h exp.h \
 stm.h obj.h cnt.h act.h loc.h ext.h rul.h emit.h
sco.o: sco.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h adv.h whr.h evt.h nam.h acode.h wht.h sco.h \
 emit.h
scr.o: scr.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h acode.h nam.h stm.h wht.h whr.h evt.h exp.h \
 obj.h cnt.h act.h loc.h ext.h stp.h scr.h emit.h
smScSema.o: smScSema.c sysdep.h types.h alan.h srcp.h lst.h dump.h \
 lmList.h alanCommon.h token.h encode.h acode.h smScan.h str.h
smScan.o: smScan.c sysdep.h types.h alan.h srcp.h lst.h dump.h \
 lmList.h alanCommon.h token.h encode.h acode.h smScan.h
smScanx.o: smScanx.c sysdep.h types.h alan.h srcp.h lst.h dump.h \
 lmList.h alanCommon.h token.h encode.h acode.h smScan.h
spa.o: spa.c spa.h
srcp.o: srcp.c alan.h sysdep.h types.h srcp.h lst.h dump.h
stm.o: stm.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h adv.h whr.h evt.h nam.h acode.h wht.h sym.h \
 elm.h res.h stx.h exp.h atr.h emit.h stm.h obj.h cnt.h act.h loc.h \
 ext.h scr.h sco.h opt.h encode.h
stp.o: stp.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h exp.h wht.h nam.h acode.h whr.h evt.h stm.h \
 obj.h cnt.h act.h loc.h ext.h stp.h emit.h
str.o: str.c sysdep.h types.h alan.h srcp.h lst.h dump.h str.h
stx.o: stx.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h adv.h whr.h evt.h nam.h acode.h wht.h sym.h \
 elm.h res.h stx.h wrd.h emit.h
sym.o: sym.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h sym.h nam.h acode.h elm.h res.h stx.h vrb.h \
 obj.h whr.h evt.h wht.h cnt.h act.h ext.h loc.h
syn.o: syn.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h acode.h adv.h whr.h evt.h nam.h wht.h syn.h \
 wrd.h emit.h
sysdep.o: sysdep.c sysdep.h
timing.o: timing.c sysdep.h timing.h
vrb.o: vrb.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h acode.h adv.h whr.h evt.h nam.h wht.h alt.h \
 act.h cnt.h sym.h elm.h res.h stx.h obj.h vrb.h emit.h
whr.o: whr.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h acode.h nam.h sym.h elm.h res.h stx.h cnt.h \
 wht.h whr.h evt.h emit.h
wht.o: wht.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h wht.h nam.h acode.h emit.h
wrd.o: wrd.c alan.h sysdep.h types.h srcp.h lst.h dump.h lmList.h \
 alanCommon.h token.h acode.h nam.h wrd.h opt.h emit.h
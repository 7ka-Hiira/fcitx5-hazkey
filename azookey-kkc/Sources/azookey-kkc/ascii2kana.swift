import Foundation

let source = """
a あ 1
i い 1
u う 1
e え 1
o お 1
va ゔぁ 2
vi ゔぃ 2
vu ゔ 2
ve ゔぇ 2
vo ゔぉ 2
vya ゔゃ 3
vyi ゔぃ 3
vyu ゔゅ 3
vye ゔぇ 3
vyo ゔょ 3
ka か 2
ki き 2
ku く 2
ke け 2
ko こ 2
kya きゃ 3
kyi きぃ 3
kyu きゅ 3
kye きぇ 3
kyo きょ 3
kwa くぁ 3
kwi くぃ 3
kwu くぅ 3
kwe くぇ 3
kwo くぉ 3
qa くぁ 2
qi くぃ 2
qe くぇ 2
qo くぉ 2
ga が 2
gi ぎ 2
gu ぐ 2
ge げ 2
go ご 2
gya ぎゃ 3
gyi ぎぃ 3
gyu ぎゅ 3
gye ぎぇ 3
gyo ぎょ 3
gwa ぐぁ 3
gwi ぐぃ 3
gwu ぐぅ 3
gwe ぐぇ 3
gwo ぐぉ 3
ca か 2
ci し 2
cu く 2
ce せ 2
co こ 2
sa さ 2
si し 2
su す 2
se せ 2
so そ 2
sya しゃ 3
syi しぃ 3
syu しゅ 3
sye しぇ 3
syo しょ 3
sha しゃ 3
shi し 3
shu しゅ 3
she しぇ 3
sho しょ 3
swa すぁ 3
swi すぃ 3
swu すぅ 3
swe すぇ 3
swo すぉ 3
za ざ 2
zi じ 2
zu ず 2
ze ぜ 2
zo ぞ 2
zya じゃ 3
zyi じぃ 3
zyu じゅ 3
zye じぇ 3
zyo じょ 3
zwu ずぅ 3
zwi ずぃ 3
zwe ずぇ 3
zwo ずぉ 3
ja じゃ 2
ji じ 2
ju じゅ 2
je じぇ 2
jo じょ 2
jya じゃ 3
jyi じぃ 3
jyu じゅ 3
jye じぇ 3
jyo じょ 3
ta た 2
ti ち 2
tu つ 2
te て 2
to と 2
tya ちゃ 3
tyu ちゅ 3
tyo ちょ 3
tha てゃ 3
thi てぃ 3
thu てゅ 3
the てぇ 3
tho てょ 3
cha ちゃ 3
chi ち 3
chu ちゅ 3
che ちぇ 3
cho ちょ 3
tsa つぁ 3
tsi つぃ 3
tsu つ 3
tse つぇ 3
tso つぉ 3
tha てゃ 3
thi てぃ 3
thu てゅ 3
the てぇ 3
tho てょ 3
twa とぁ 3
twi とぃ 3
twu とぅ 3
twe とぇ 3
two とぉ 3
da だ 2
di ぢ 2
du づ 2
de で 2
do ど 2
dya ぢゃ 3
dyi ぢぃ 3
dyu ぢゅ 3
dye ぢぇ 3
dyo ぢょ 3
dwa どぁ 3
dwi どぃ 3
dwu どぅ 3
dwe どぇ 3
dwo どぉ 3
na な 2
ni に 2
nu ぬ 2
ne ね 2
no の 2
nya にゃ 3
nyi にぃ 3
nyu にゅ 3
nye にぇ 3
nyo にょ 3
ha は 2
hi ひ 2
6 [ra]         0x0000614b0c0f13c2 ascii2kana(ascii:) + 
hu ふ 2
he へ 2
ho ほ 2
hya ひゃ 3
hyi ひぃ 3
hyu ひゅ 3
hye ひぇ 3
hyo ひょ 3
fa ふぁ 2
fi ふぃ 2
fu ふ 2
fe ふぇ 2
fo ふぉ 2
fya ふゃ 3
fyi ふぃ 3
fyu ふゅ 3
fye ふぇ 3
fyo ふょ 3
hwa ふぁ 3
hwi ふぃ 3
hwu ふ 3
hwe ふぇ 3
hwo ふぉ 3
hwya ふゃ 4
hwyi ふぃ 4
hwyu ふゅ 4
hwye ふぇ 4
hwyo ふょ 4
ba ば 2
bi び 2
bu ぶ 2
be べ 2
bo ぼ 2
bya びゃ 3
byi びぃ 3
byu びゅ 3
bye びぇ 3
byo びょ 3
pa ぱ 2
pi ぴ 2
pu ぷ 2
pe ぺ 2
po ぽ 2
pya ぴゃ 3
pyi ぴぃ 3
pyu ぴゅ 3
pye ぴぇ 3
pyo ぴょ 3
pha ふぁ 3
phi ふぃ 3
phu ふ 3
phe ふぇ 3
pho ふぉ 3
ma ま 2
mi み 2
mu む 2
me め 2
mo も 2
mya みゃ 3
myi みぃ 3
myu みゅ 3
mye みぇ 3
myo みょ 3
ya や 2
yi 𛀆 2
yu ゆ 2
ye 𛀁 2
yo よ 2
ra ら 2
ri り 2
ru る 2
re れ 2
ro ろ 2
rya りゃ 3
ryi りぃ 3
ryu りゅ 3
rye りぇ 3
ryo りょ 3
wa わ 2
wi うぃ 2
wu う 2
we うぇ 2
wo を 2
wha うぁ 3
whi うぃ 3
whu う 3
whe うぇ 3
who うぉ 3
wyi ゐ 3
wyu 𛄟 3
wye ゑ 3
nn ん 2
la ぁ 2
li ぃ 2
lu ぅ 2
le ぇ 2
lo ぉ 2
lka ゕ 3
lke ゖ 3
lko 𛄲 3
lya ゃ 3
lyu ゅ 3
lyo ょ 3
lwa ゎ 3
lwo 𛅒 3
lwyi 𛅐 4
lwye 𛅑 4
ltu っ 3
ltsu っ 4
xa ぁ 2
xi ぃ 2
xu ぅ 2
xe ぇ 2
xo ぉ 2
xka	ゕ 3
xke	ゖ 3
xya ゃ 3
xyu ゅ 3
xyo ょ 3
xwa ゎ 3
xwo 𛅒 3
xwyi 𛅐 4
xwye 𛅑 4
xtu っ 3
xtsu っ 4
kk っ 1
cc っ 1
qq っ 1
gg っ 1
ss っ 1
zz っ 1
jj っ 1
tt っ 1
dd っ 1
hh っ 1
ff っ 1
bb っ 1
pp っ 1
mm っ 1
yy っ 1
rr っ 1
ww っ 1
vv っ 1
ll っ 1
xx っ 1
nk ん 1
ng ん 1
nc ん 1
nq ん 1
ns ん 1
nz ん 1
nj ん 1
nt ん 1
nd ん 1
nh ん 1
nf ん 1
nb ん 1
np ん 1
nm ん 1
ny ん 1
nr ん 1
nw ん 1
nv ん 1
nl ん 1
nx ん 1
zh ← 2
zj ↓ 2
zk → 2
zn ↑ 2
z/ ・ 2
z. … 2
z, ‥ 2
z- 〜 2
z[ 『 2
z] 』 2
z' ゙ 2
z'' ゚ 2
z@ ゚ 2
z@@ ゙ 2
[ 「 1
] 」 1
- ー 1
. 。 1
, 、 1
/ ・ 1
: ： 1
; ； 1
! ！ 1
? ？ 1
( （ 1
) ） 1
[ ［ 1
] ］ 1
{ ｛ 1
} ｝ 1
" ” 1
' ’ 1
` ｀ 1
^ ＾ 1
_ ＿ 1
~ 〜 1
n\u{3} ん\u{3} 2
\u{3} \u{3} 1
"""

func asciiHiraganaTable(source: String) -> [String: (String, Int)] {
    var dictionary = [String: (String, Int)]()
    let lines = source.split(separator: "\n")
    for line in lines {
        let components = line.split(separator: " ")
        if components.count != 3 {
            continue
        }
        let romaji = String(components[0])
        let hiragana = String(components[1])
        let consumes = Int(components[2]) ?? 0
        dictionary[romaji] = (hiragana, consumes)
    }
    return dictionary
}

func ascii2hiragana(ascii: String) -> String {
    let table = asciiHiraganaTable(source: source)
    let ascii = ascii + "\u{3}"
    var kana = ""
    var index = ascii.startIndex
    while index < ascii.endIndex {
        var found = false
        for j in (1...3).reversed() {
            let endIndex = ascii.index(index, offsetBy: j, limitedBy: ascii.endIndex) ?? ascii.endIndex
            let asciiSubstring = ascii[index..<endIndex]
            if let (hiragana, consumes) = table[String(asciiSubstring)] {
                kana += hiragana
                index = ascii.index(index, offsetBy: consumes)
                found = true
                break
            }
        }
        if !found {
            kana += String(ascii[index])
            index = ascii.index(after: index)
        }
    }
    let _ = kana.popLast()
    return kana
}

@_silgen_name("kkc_ascii2hiragana")
public func kkc_ascii2hiragana(asciiPtr: UnsafePointer<Int8>?, configPtr: UnsafePointer<KkcConfig>?) -> UnsafeMutablePointer<Int8>? {
    guard let asciiPtr = asciiPtr else {
        return nil
    }

    let asciiString = String(cString: asciiPtr)
    let kana = ascii2hiragana(ascii: asciiString)
    return strdup(kana)
}
@_silgen_name("kkc_free_ascii2hiragana")
public func kkc_free_ascii2hiragana(ptr: UnsafeMutablePointer<Int8>?) {
    guard let ptr = ptr else {
        return
    }
    free(ptr)
}

package bignum

import (
	"bytes"
	"fmt"
)

type Bignum struct {
	bytes.Buffer
	negative bool
}

func New() Bignum {
	b := Bignum{
		negative: false,
	}
	return b
}

func FromInt(s uint32) Bignum {
	var buff [4]byte
	buff[0] = byte(s & 0x000000ff)
	buff[1] = byte((s & 0x0000ff00) >> 8)
	buff[2] = byte((s & 0x00ff0000) >> 16)
	buff[3] = byte((s & 0xff000000) >> 24)
	bn := Bignum{
		negative: false,
	}
	bn.Write(bytes.TrimRightFunc(buff[:], func(r rune) bool {
		return r == 0
	}))
	return bn
}

func (bn Bignum) String() string {
	if bn.Len() == 0 {
		return "{0: []}"
	}
	str := fmt.Sprintf("{%d: [", bn.Len())
	end := bn.Len() - 1
	for i := 0; i < end; i++ {
		str += fmt.Sprintf("%d, ", bn.Bytes()[i])
	}
	str += fmt.Sprintf("%d], 0x[", bn.Bytes()[end])
	for i := 0; i < end; i++ {
		str += fmt.Sprintf("%x, ", bn.Bytes()[i])
	}
	return str + fmt.Sprintf("%x]}", bn.Bytes()[end])
}

func (bn Bignum) Dump() {
	fmt.Print(bn)
}

func (bn Bignum) IsZero() bool {
	return bn.Len() == 0 || (bn.Len() == 1 && bn.Bytes()[0] == 0)
}

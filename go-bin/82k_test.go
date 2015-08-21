package main

import (
	"math/big"
	"testing"
)

func init() {
	initBaseConvert(40, 5)
}

func TestSetDeepCopies(t *testing.T) {
	a := big.NewInt(7)
	var b big.Int
	b.Set(a)
	a.SetInt64(9)
	if a.Cmp(&b) == 0 {
		t.Fatalf("fsck")
	}
}

func TestBaseConvert(t *testing.T) {
	bn := big.NewInt(2) // binary 10
	bn2 := baseConvert(bn)
	if bn2.Int64() != 5 {
		t.Fatalf("bn2 = %s, expected 5", bn2)
	}
}

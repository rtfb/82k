package main

import (
	"fmt"

	"./bignum"
)

func main() {
	bn := bignum.New()
	bn.WriteByte('a')
	fmt.Printf("hi, %s\n", bn)
	bn2 := bignum.FromInt(258)
	fmt.Printf("hi, %s\n", bn2)
}

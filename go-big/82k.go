package main

import (
	"flag"
	"fmt"
	"log"
	"math/big"
	"os"
	"runtime/pprof"
)

var (
	mulLUT     []big.Int
	cpuprofile = flag.String("cpuprofile", "", "write cpu profile to file")
)

func initBaseConvert(size, base uint32) {
	multiplier := big.NewInt(1)
	i := uint32(0)
	mulLUT = make([]big.Int, size, size)
	for i < size {
		mulLUT[i].Set(multiplier)
		multiplier.Mul(multiplier, big.NewInt(int64(base)))
		i += 1
	}
}

func baseConvert(n *big.Int) *big.Int {
	r := big.NewInt(0)
	m := 0
	for i := len(n.Bytes()) - 1; i >= 0; i -= 1 {
		for mask := uint8(1); mask != 0; mask = mask << 1 {
			if n.Bytes()[i]&mask != 0 {
				r.Add(r, &mulLUT[m])
			}
			m += 1
		}
	}
	return r
}

func checkBase(n *big.Int, base int) bool {
	var work big.Int
	work.Set(n)
	zero := big.NewInt(0)
	bigBase := big.NewInt(int64(base))
	var digit big.Int
	for work.Cmp(zero) != 0 {
		work.DivMod(&work, bigBase, &digit)
		if digit.Int64() > 1 {
			return false
		}
	}
	return true
}

func search() {
	n5 := big.NewInt(1)
	baseCap := 5
	for len(n5.Bytes()) < 4 {
		n := baseConvert(n5)
		//println(n5.String(), n.String())
		base := baseCap
		for base > 2 {
			if !checkBase(n, base) {
				break
			}
			base -= 1
		}
		if base == 2 {
			fmt.Printf("covers all bases from 2 to %d: %s\n", baseCap, n)
		}
		n5.Add(n5, big.NewInt(1))
	}
}

func main() {
	flag.Parse()
	if *cpuprofile != "" {
		f, err := os.Create(*cpuprofile)
		if err != nil {
			log.Fatal(err)
		}
		pprof.StartCPUProfile(f)
		defer pprof.StopCPUProfile()
	}
	initBaseConvert(40, 5)
	search()
	//n := big.NewInt(0)
	//n.Exp(big.NewInt(10), big.NewInt(375), nil)
	//n.Add(n, big.NewInt(17))
	//fmt.Printf("n = %s\n", n)
}

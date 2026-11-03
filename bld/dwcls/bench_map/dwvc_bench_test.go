package main

import (
	"testing"

	dwvc "github.com/dwyco/dwvc"
)

func BenchmarkDwvcEmptyCreateDestroy(b *testing.B) {
	for i := 0; i < b.N; i++ {
		m := make(map[dwvc.Vc]dwvc.Vc)
		_ = m
	}
}

func BenchmarkDwvcInsert(b *testing.B) {
	for _, n := range []int{1, 2, 3, 4, 5} {
		n := n
		b.Run("n="+itoa(n), func(b *testing.B) {
			keys := make([]dwvc.Vc, n)
			for k := 0; k < n; k++ {
				keys[k] = dwvc.VcString("key" + itoa(k))
			}
			b.ResetTimer()
			for i := 0; i < b.N; i++ {
				m := make(map[dwvc.Vc]dwvc.Vc)
				for k := 0; k < n; k++ {
					m[keys[k]] = keys[k]
				}
				_ = len(m)
			}
		})
	}
}

func BenchmarkDwvcLookup(b *testing.B) {
	for _, n := range []int{1, 2, 3, 4, 5} {
		n := n
		b.Run("n="+itoa(n), func(b *testing.B) {
			keys := make([]dwvc.Vc, n)
			for k := 0; k < n; k++ {
				keys[k] = dwvc.VcString("key" + itoa(k))
			}
			m := make(map[dwvc.Vc]dwvc.Vc)
			for k := 0; k < n; k++ {
				m[keys[k]] = keys[k]
			}
			b.ResetTimer()
			sink := 0
			for i := 0; i < b.N; i++ {
				for j := 0; j < n; j++ {
					v := m[keys[j%n]]
					sink += int(v.Type())
				}
			}
			_ = sink
		})
	}
}

func BenchmarkDwvcLookupMiss(b *testing.B) {
	for _, n := range []int{1, 2, 4, 8, 16, 32} {
		n := n
		b.Run("n="+itoa(n), func(b *testing.B) {
			keys := make([]dwvc.Vc, n)
			for k := 0; k < n; k++ {
				keys[k] = dwvc.VcString("key" + itoa(k))
			}
			miss := make([]dwvc.Vc, 5)
			for j := 0; j < 5; j++ {
				miss[j] = dwvc.VcString("missing" + itoa(j))
			}
			m := make(map[dwvc.Vc]dwvc.Vc)
			for k := 0; k < n; k++ {
				m[keys[k]] = keys[k]
			}
			b.ResetTimer()
			sink := 0
			for i := 0; i < b.N; i++ {
				for j := 0; j < 5; j++ {
					v := m[miss[j]]
					sink += int(v.Type())
				}
			}
			_ = sink
		})
	}
}

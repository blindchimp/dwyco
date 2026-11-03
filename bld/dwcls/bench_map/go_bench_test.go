package main

import (
	"testing"
)

func BenchmarkGoEmptyCreateDestroy(b *testing.B) {
	for i := 0; i < b.N; i++ {
		m := make(map[int]int)
		_ = m
	}
}

func BenchmarkGoInsert(b *testing.B) {
	for _, n := range []int{1, 2, 3, 4, 5} {
		n := n
		b.Run("n="+itoa(n), func(b *testing.B) {
			keys := make([]int, n)
			vals := make([]int, n)
			for k := 0; k < n; k++ {
				keys[k] = k
				vals[k] = k*3 + 1
			}
			b.ResetTimer()
			for i := 0; i < b.N; i++ {
				m := make(map[int]int)
				for k := 0; k < n; k++ {
					m[keys[k]] = vals[k]
				}
				_ = len(m)
			}
		})
	}
}

func BenchmarkGoLookup(b *testing.B) {
	for _, n := range []int{1, 2, 3, 4, 5} {
		n := n
		b.Run("n="+itoa(n), func(b *testing.B) {
			keys := make([]int, n)
			vals := make([]int, n)
			for k := 0; k < n; k++ {
				keys[k] = k
				vals[k] = k*3 + 1
			}
			m := make(map[int]int)
			for k := 0; k < n; k++ {
				m[keys[k]] = vals[k]
			}
			b.ResetTimer()
			sink := 0
			for i := 0; i < b.N; i++ {
				for j := 0; j < n; j++ {
					sink += m[keys[j%n]]
				}
			}
			_ = sink
		})
	}
}

func itoa(n int) string {
	if n == 0 {
		return "0"
	}
	digits := []byte{}
	for n > 0 {
		digits = append([]byte{byte('0' + n%10)}, digits...)
		n /= 10
	}
	return string(digits)
}

func BenchmarkGoStringEmptyCreateDestroy(b *testing.B) {
	for i := 0; i < b.N; i++ {
		m := make(map[string]string)
		_ = m
	}
}

func BenchmarkGoStringInsert(b *testing.B) {
	for _, n := range []int{1, 2, 3, 4, 5} {
		n := n
		b.Run("n="+itoa(n), func(b *testing.B) {
			keys := make([]string, n)
			for k := 0; k < n; k++ {
				keys[k] = "key" + itoa(k)
			}
			b.ResetTimer()
			for i := 0; i < b.N; i++ {
				m := make(map[string]string)
				for k := 0; k < n; k++ {
					m[keys[k]] = keys[k]
				}
				_ = len(m)
			}
		})
	}
}

func BenchmarkGoStringLookup(b *testing.B) {
	for _, n := range []int{1, 2, 3, 4, 5} {
		n := n
		b.Run("n="+itoa(n), func(b *testing.B) {
			keys := make([]string, n)
			for k := 0; k < n; k++ {
				keys[k] = "key" + itoa(k)
			}
			m := make(map[string]string)
			for k := 0; k < n; k++ {
				m[keys[k]] = keys[k]
			}
			b.ResetTimer()
			sink := 0
			for i := 0; i < b.N; i++ {
				for j := 0; j < n; j++ {
					sink += len(m[keys[j%n]])
				}
			}
			_ = sink
		})
	}
}

func BenchmarkGoLookupMiss(b *testing.B) {
	for _, n := range []int{1, 2, 4, 8, 16, 32} {
		n := n
		b.Run("n="+itoa(n), func(b *testing.B) {
			keys := make([]int, n)
			vals := make([]int, n)
			for k := 0; k < n; k++ {
				keys[k] = k
				vals[k] = k*3 + 1
			}
			miss := make([]int, 5)
			for j := 0; j < 5; j++ {
				miss[j] = n + 1 + j
			}
			m := make(map[int]int)
			for k := 0; k < n; k++ {
				m[keys[k]] = vals[k]
			}
			b.ResetTimer()
			sink := 0
			for i := 0; i < b.N; i++ {
				for j := 0; j < 5; j++ {
					sink += m[miss[j]]
				}
			}
			_ = sink
		})
	}
}

func BenchmarkGoStringLookupMiss(b *testing.B) {
	for _, n := range []int{1, 2, 4, 8, 16, 32} {
		n := n
		b.Run("n="+itoa(n), func(b *testing.B) {
			keys := make([]string, n)
			for k := 0; k < n; k++ {
				keys[k] = "key" + itoa(k)
			}
			miss := make([]string, 5)
			for j := 0; j < 5; j++ {
				miss[j] = "missing" + itoa(j)
			}
			m := make(map[string]string)
			for k := 0; k < n; k++ {
				m[keys[k]] = keys[k]
			}
			b.ResetTimer()
			sink := 0
			for i := 0; i < b.N; i++ {
				for j := 0; j < 5; j++ {
					sink += len(m[miss[j]])
				}
			}
			_ = sink
		})
	}
}

package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"

	"github.com/dwyco/dwvc"
)

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		v, err := dwvc.Decode([]byte(line))
		if err != nil {
			fmt.Fprintf(os.Stderr, "error: %v\n", err)
			continue
		}
		fmt.Println(v.String())
	}
}

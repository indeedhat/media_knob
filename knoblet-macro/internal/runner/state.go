package runner

import "fmt"

func state(angle int16, side, base bool) string {
	return fmt.Sprintf(`%d;%t;%t`, angle, side, base)
}

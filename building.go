package building

// GCC 9 以下的版本无法编译通过!!!
// 请通过gcc -v命令检查当前的GCC版本

/*
#cgo CFLAGS: -O3
#cgo CXXFLAGS: -I${SRCDIR}/include -std=c++17 -fPIC -ffast-math -O3 -DNDEBUG -DCGO -DUNICODE
#cgo LDFLAGS: -O3

#include <stdio.h>
#include <stdlib.h>
#include "go_bridge.h"
*/
import "C"
import (
	"path/filepath"
	"unsafe"
)

func CAlbcTest() {
	gamedata, _ := filepath.Abs("./building_data.json")
	playerdata, _ := filepath.Abs("./player_data.json")

	gamedataPath := C.CString(gamedata)
	playerdataPath := C.CString(playerdata)
	C.AlbcTest(gamedataPath, playerdataPath)
	C.free(unsafe.Pointer(gamedataPath))
	C.free(unsafe.Pointer(playerdataPath))
}

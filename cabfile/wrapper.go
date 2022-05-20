package cabfile

/*
#cgo pkg-config: libmspack
#include "extractor.h"

extern void fileCallbackGlue(char *filename, void *data, int data_len);

static inline void InitCallback(){
    SetFileCallback(fileCallbackGlue);
}

*/
import "C"
import (
	"fmt"
	"io"
	"log"
	"unsafe"
)

var _ff FileFunc

//export fileCallbackGlue
func fileCallbackGlue(n *C.char, d unsafe.Pointer, dataLen C.int) {
	name := C.GoString(n)
	data := C.GoBytes(d, dataLen)
	err := _ff(name, data)
	if err != nil {
		log.Printf("failed file callback %v", err)
	}
}

func init() {
	C.InitCallback()
}

type FileFunc func(name string, data []byte) error

func Extract(r io.Reader, ff FileFunc) error {
	data, err := io.ReadAll(r)
	if err != nil {
		return err
	}
	_ff = ff
	ret := C.extract(
		C.CString(string(data)),
		C.size_t(len(data)),
	)
	if ret != 0 {
		return fmt.Errorf("failed extraction exit: %d", ret)
	}
	return nil
}

#pragma once
#include "Types.h"
#include "Pointer.h"
#include "Stack.h"
#include "Memory.h"
#include "Class.h"

class vFrame {
public:
	V<vBYTE>	_program;
	V<vStack>	_stack;
	V<vMemory>	_local;
	V<vClass>	_class;
	vULONG		_pc;
	bool		_hasRet;

	vFrame(
		vMETHOD* method,
		const V<vClass>& classFile,
		size_t maxStackItems = 512,
		size_t maxLocals = 512
	) {
		_stack = VMAKE(vStack, maxStackItems * sizeof(vCOMMON));
		_local = VMAKE(vMemory, maxLocals);
		_pc = 0;
		_class = classFile;
		_program = classFile->getCode(method);
		V<vUTF8BODY> desc = classFile->toString(method->desc);
		_hasRet = false;
		if (desc.IsValid()) {
			size_t len = strlen((const char*)desc->s.Real());
			_hasRet = desc->s[len - 1] != 'V';
		}
	}

	~vFrame() {
		_stack.Release();
		_local.Release();
	}

	vBYTE* fetch() {
		return _program.Real() + pc();
	}

	vULONG& pc() {
		return _pc;
	}

	vULONG incrpc(size_t step) {
		return (_pc) += step;
	}
};
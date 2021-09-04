#pragma once
#include "Types.h"
#include "Pointer.h"
#include "Stack.h"
#include "Memory.h"
#include "Class.h"

namespace jaether {

	class vFrame {
	public:
		V<vBYTE>	_program;
		V<vStack>	_stack;
		V<vMemory>	_local;
		V<vClass>	_class;
		vULONG		_pc;
		bool		_returns;

		vFrame(
			vContext* ctx,
			vMETHOD* method,
			const V<vClass>& classFile,
			size_t maxStackItems = 64,
			size_t maxLocals = 512
		) {
			_stack = VMAKE(vStack, ctx, ctx, maxStackItems * sizeof(vCOMMON));
			_local = VMAKE(vMemory, ctx, ctx, maxLocals);
			_pc = 0;
			_class = classFile;
			_program = classFile(ctx)->getCode(ctx, method);
			if (!_program.isValid()) {
				DPRINTF("Method %s/%s:%s has no code\n",
					classFile(ctx)->getName(ctx),
					classFile(ctx)->toString(ctx, method->name)(ctx)->s(ctx),
					classFile(ctx)->toString(ctx, method->desc)(ctx)->s(ctx)
				);
			}
			V<vUTF8BODY> desc = classFile(ctx)->toString(ctx, method->desc);
			_returns = false;
			if (desc.isValid()) {
				size_t len = strlen((const char*)desc(ctx)->s.real(ctx));
				_returns = desc(ctx)->s( ctx,  len - 1) != 'V';
			}
		}

		~vFrame() {
		}

		void destroy(vContext* ctx) {
			_stack(ctx)->destroy(ctx);
			_stack.release(ctx);
			_local(ctx)->destroy(ctx);
			_local.release(ctx);
		}

		vBYTE* fetch(vContext* ctx) {
			return _program.real(ctx) + pc();
		}

		vULONG& pc() {
			return _pc;
		}

		vULONG& incrpc(size_t step) {
			return (_pc) += step;
		}
	};

}
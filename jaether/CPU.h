#pragma once
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <cmath>
#include "Opcodes.h"
#include "Pointer.h"
#include "Types.h"
#include "Stack.h"
#include "Memory.h"
#include "Frame.h"
#include "Class.h"
#include <map>
#include <string>

class vCPU {
	bool _running = true;
	std::map<std::string, V<vClass>> _classes;
	std::map<std::string, vNATIVE> _natives;
public:
	vCPU();

	V<vClass> load(vContext* ctx, const std::string& s, const std::string& parent = "");
	void addNative(const std::string& path, const std::string& desc, const vNATIVE& native);

	bool active() const;

	void run(vContext* ctx, const V<vFrame>& frame);

	size_t execute(vContext* ctx, const V<vFrame>& frame);
	size_t sub_execute(vContext* ctx, const V<vFrame>& frame);

	template<class T> T read(vBYTE* ip) const {
		return *(T*)ip;
	}

	vUSHORT readUSI(vBYTE* ip) const;

	vUINT readUI(vBYTE* ip) const;

	template<class T> vINT compare(T a, T b) {
		if (a == b) return 0;
		if (a > b) return 1;
		return -1;
	}
};
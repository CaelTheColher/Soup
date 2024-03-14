#pragma once

#include "base.hpp"
#include "fwd.hpp"
#include "type_traits.hpp"

#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace soup
{
	class WasmVm;

	using wasm_ffi_func_t = void(*)(WasmVm&);

	union WasmValue
	{
		int32_t i32;
		int64_t i64;
		float f32;
		double f64;

		WasmValue() : i64(0) {}
		WasmValue(int32_t i32) : i32(i32) {}
		WasmValue(uint32_t u32) : WasmValue(static_cast<int32_t>(u32)) {}
		WasmValue(int64_t i64) : i64(i64) {}
		WasmValue(uint64_t u64) : WasmValue(static_cast<int64_t>(u64)) {}
		WasmValue(float f32) : f32(f32) {}
		WasmValue(double f64) : f64(f64) {}

		template <typename T, SOUP_RESTRICT(std::is_same_v<T, size_t>)>
		WasmValue(T ptr) : i32(static_cast<int32_t>(ptr)) {}
	};

	struct WasmScript
	{
		struct FunctionType
		{
			size_t num_parameters;
			size_t num_results;
		};

		struct FunctionImport
		{
			std::string module_name;
			std::string function_name;
			wasm_ffi_func_t ptr;
		};

		uint8_t* memory = nullptr;
		size_t memory_size = 0;
		size_t last_alloc = -1;
		std::vector<size_t> functions{}; // (function_index - function_imports.size()) -> type_index
		std::vector<FunctionType> types{};
		std::vector<FunctionImport> function_imports{};
		std::vector<int32_t> globals{};
		std::unordered_map<std::string, size_t> export_map{};
		std::vector<std::string> code{};
		std::vector<size_t> elements{};
		bool memory64 = false;

		~WasmScript() noexcept;

		bool load(const std::string& data);
		bool load(Reader& r);

		[[nodiscard]] FunctionImport* getImportedFunction(const std::string& module_name, const std::string& function_name) noexcept;
		[[nodiscard]] const std::string* getExportedFuntion(const std::string& name) const noexcept;

		[[nodiscard]] size_t allocateMemory(size_t len) noexcept;

		template <typename T>
		[[nodiscard]] T* getMemory(size_t ptr) noexcept
		{
			SOUP_IF_UNLIKELY (ptr + sizeof(T) >= memory_size)
			{
				return nullptr;
			}
			return (T*)&memory[ptr];
		}

		template <typename T>
		[[nodiscard]] T* getMemory(WasmValue base, size_t offset = 0) noexcept
		{
			if (memory64)
			{
				return getMemory<T>(static_cast<uint64_t>(base.i64) + offset);
			}
			return getMemory<T>(static_cast<uint32_t>(base.i32) + offset);
		}

		bool setMemory(size_t ptr, const void* src, size_t len) noexcept;
		bool setMemory(WasmValue ptr, const void* src, size_t len) noexcept;

		void linkWasiPreview1() noexcept;

		[[nodiscard]] size_t readUPTR(Reader& r) const noexcept;
	};

	class WasmVm
	{
	public:
		std::stack<WasmValue> stack;
		std::vector<WasmValue> locals;
		WasmScript& script;

		WasmVm(WasmScript& script)
			: script(script)
		{
		}

		bool run(const std::string& data) SOUP_EXCAL;
		bool run(Reader& r) SOUP_EXCAL;

	private:
		struct CtrlFlowEntry
		{
			size_t position;
			size_t stack_size;
		};

		bool skipOverBranch(Reader& r, uint32_t depth = 0) SOUP_EXCAL;
		[[nodiscard]] bool doBranch(Reader& r, uint32_t depth, std::stack<CtrlFlowEntry>& ctrlflow) SOUP_EXCAL;
		[[nodiscard]] bool doCall(size_t type_index, size_t function_index) SOUP_EXCAL;
		void pushIPTR(size_t ptr) SOUP_EXCAL;
	};
}

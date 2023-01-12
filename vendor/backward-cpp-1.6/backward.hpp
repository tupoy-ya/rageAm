/*
 * backward.hpp
 * Copyright 2013 Google Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef H_6B9572DA_A64B_49E6_B234_051480991C89
#define H_6B9572DA_A64B_49E6_B234_051480991C89

#ifndef __cplusplus
#error "It's not going to compile without a C++ compiler..."
#endif

#if defined(BACKWARD_CXX11)
#elif defined(BACKWARD_CXX98)
#else
#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1800)
#define BACKWARD_CXX11
#define BACKWARD_ATLEAST_CXX11
#define BACKWARD_ATLEAST_CXX98
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#define BACKWARD_ATLEAST_CXX17
#endif
#else
#define BACKWARD_CXX98
#define BACKWARD_ATLEAST_CXX98
#endif
#endif

 // You can define one of the following (or leave it to the auto-detection):
 //
 // #define BACKWARD_SYSTEM_LINUX
 //	- specialization for linux
 //
 // #define BACKWARD_SYSTEM_DARWIN
 //	- specialization for Mac OS X 10.5 and later.
 //
 // #define BACKWARD_SYSTEM_WINDOWS
 //  - specialization for Windows (Clang 9 and MSVC2017)
 //
 // #define BACKWARD_SYSTEM_UNKNOWN
 //	- placebo implementation, does nothing.
 //
#if defined(BACKWARD_SYSTEM_LINUX)
#elif defined(BACKWARD_SYSTEM_DARWIN)
#elif defined(BACKWARD_SYSTEM_UNKNOWN)
#elif defined(BACKWARD_SYSTEM_WINDOWS)
#else
#if defined(__linux) || defined(__linux__)
#define BACKWARD_SYSTEM_LINUX
#elif defined(__APPLE__)
#define BACKWARD_SYSTEM_DARWIN
#elif defined(_WIN32)
#define BACKWARD_SYSTEM_WINDOWS
#else
#define BACKWARD_SYSTEM_UNKNOWN
#endif
#endif

#define NOINLINE __attribute__((noinline))

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <new>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>
#include <exception>
#include <iterator>

#if defined(BACKWARD_SYSTEM_WINDOWS)

#include <condition_variable>
#include <mutex>
#include <thread>

#include <basetsd.h>
typedef SSIZE_T ssize_t;

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winnt.h>

#include <psapi.h>
#include <signal.h>

#ifndef __clang__
#undef NOINLINE
#define NOINLINE __declspec(noinline)
#endif

#ifdef _MSC_VER
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dbghelp.lib")
#endif

// Comment / packing is from stackoverflow:
// https://stackoverflow.com/questions/6205981/windows-c-stack-trace-from-a-running-app/28276227#28276227
// Some versions of imagehlp.dll lack the proper packing directives themselves
// so we need to do it.
#pragma pack(push, before_imagehlp, 8)
#include <imagehlp.h>
#pragma pack(pop, before_imagehlp)

// TODO maybe these should be undefined somewhere else?
#undef BACKWARD_HAS_UNWIND
#undef BACKWARD_HAS_BACKTRACE
#if BACKWARD_HAS_PDB_SYMBOL == 1
#else
#undef BACKWARD_HAS_PDB_SYMBOL
#define BACKWARD_HAS_PDB_SYMBOL 1
#endif

#endif

#include <unordered_map>
#include <utility> // for std::swap
namespace backward {
	namespace details {
		template <typename K, typename V> struct hashtable {
			typedef std::unordered_map<K, V> type;
		};
		using std::move;
	} // namespace details
} // namespace backward

namespace backward {
	namespace details {
#if defined(BACKWARD_SYSTEM_WINDOWS)
		const char kBackwardPathDelimiter[] = ";";
#else
		const char kBackwardPathDelimiter[] = ":";
#endif
	} // namespace details
} // namespace backward

namespace backward {

	namespace system_tag {
		struct linux_tag; // seems that I cannot call that "linux" because the name
		// is already defined... so I am adding _tag everywhere.
		struct darwin_tag;
		struct windows_tag;
		struct unknown_tag;

#if defined(BACKWARD_SYSTEM_LINUX)
		typedef linux_tag current_tag;
#elif defined(BACKWARD_SYSTEM_DARWIN)
		typedef darwin_tag current_tag;
#elif defined(BACKWARD_SYSTEM_WINDOWS)
		typedef windows_tag current_tag;
#elif defined(BACKWARD_SYSTEM_UNKNOWN)
		typedef unknown_tag current_tag;
#else
#error "May I please get my system defines?"
#endif
	} // namespace system_tag

	namespace trace_resolver_tag {
#if defined(BACKWARD_SYSTEM_LINUX)
		struct libdw;
		struct libbfd;
		struct libdwarf;
		struct backtrace_symbol;

#if BACKWARD_HAS_DW == 1
		typedef libdw current;
#elif BACKWARD_HAS_BFD == 1
		typedef libbfd current;
#elif BACKWARD_HAS_DWARF == 1
		typedef libdwarf current;
#elif BACKWARD_HAS_BACKTRACE_SYMBOL == 1
		typedef backtrace_symbol current;
#else
#error "You shall not pass, until you know what you want."
#endif
#elif defined(BACKWARD_SYSTEM_DARWIN)
		struct backtrace_symbol;

#if BACKWARD_HAS_BACKTRACE_SYMBOL == 1
		typedef backtrace_symbol current;
#else
#error "You shall not pass, until you know what you want."
#endif
#elif defined(BACKWARD_SYSTEM_WINDOWS)
		struct pdb_symbol;
#if BACKWARD_HAS_PDB_SYMBOL == 1
		typedef pdb_symbol current;
#else
#error "You shall not pass, until you know what you want."
#endif
#endif
	} // namespace trace_resolver_tag

	namespace details {

		template <typename T> struct rm_ptr { typedef T type; };

		template <typename T> struct rm_ptr<T*> { typedef T type; };

		template <typename T> struct rm_ptr<const T*> { typedef const T type; };

		template <typename R, typename T, R(*F)(T)> struct deleter {
			template <typename U> void operator()(U& ptr) const { (*F)(ptr); }
		};

		template <typename T> struct default_delete {
			void operator()(T& ptr) const { delete ptr; }
		};

		template <typename T, typename Deleter = deleter<void, void*, &::free>>
		class handle {
			struct dummy;
			T _val;
			bool _empty;

			handle(const handle&) = delete;
			handle& operator=(const handle&) = delete;

		public:
			~handle() {
				if (!_empty) {
					Deleter()(_val);
				}
			}

			explicit handle() : _val(), _empty(true) {}
			explicit handle(T val) : _val(val), _empty(false) {
				if (!_val)
					_empty = true;
			}

			handle(handle&& from) : _empty(true) { swap(from); }
			handle& operator=(handle&& from) {
				swap(from);
				return *this;
			}

			void reset(T new_val) {
				handle tmp(new_val);
				swap(tmp);
			}

			void update(T new_val) {
				_val = new_val;
				_empty = !static_cast<bool>(new_val);
			}

			operator const dummy* () const {
				if (_empty) {
					return nullptr;
				}
				return reinterpret_cast<const dummy*>(_val);
			}
			T get() { return _val; }
			T release() {
				_empty = true;
				return _val;
			}
			void swap(handle& b) {
				using std::swap;
				swap(b._val, _val);     // can throw, we are safe here.
				swap(b._empty, _empty); // should not throw: if you cannot swap two
				// bools without throwing... It's a lost cause anyway!
			}

			T& operator->() { return _val; }
			const T& operator->() const { return _val; }

			typedef typename rm_ptr<T>::type& ref_t;
			typedef const typename rm_ptr<T>::type& const_ref_t;
			ref_t operator*() { return *_val; }
			const_ref_t operator*() const { return *_val; }
			ref_t operator[](size_t idx) { return _val[idx]; }

			// Watch out, we've got a badass over here
			T* operator&() {
				_empty = false;
				return &_val;
			}
		};

		// Default demangler implementation (do nothing).
		template <typename TAG> struct demangler_impl {
			static std::string demangle(const char* funcname) { return funcname; }
		};

		struct demangler : public demangler_impl<system_tag::current_tag> {};

		// Split a string on the platform's PATH delimiter.  Example: if delimiter
		// is ":" then:
		//   ""              --> []
		//   ":"             --> ["",""]
		//   "::"            --> ["","",""]
		//   "/a/b/c"        --> ["/a/b/c"]
		//   "/a/b/c:/d/e/f" --> ["/a/b/c","/d/e/f"]
		//   etc.
		inline std::vector<std::string> split_source_prefixes(const std::string& s) {
			std::vector<std::string> out;
			size_t last = 0;
			size_t next = 0;
			size_t delimiter_size = sizeof(kBackwardPathDelimiter) - 1;
			while ((next = s.find(kBackwardPathDelimiter, last)) != std::string::npos) {
				out.push_back(s.substr(last, next - last));
				last = next + delimiter_size;
			}
			if (last <= s.length()) {
				out.push_back(s.substr(last));
			}
			return out;
		}

	} // namespace details

	/*************** A TRACE ***************/

	struct Trace {
		void* addr;
		size_t idx;

		Trace() : addr(nullptr), idx(0) {}

		explicit Trace(void* _addr, size_t _idx) : addr(_addr), idx(_idx) {}
	};

	struct ResolvedTrace : public Trace {

		struct SourceLoc {
			std::string function;
			std::string filename;
			unsigned line;
			unsigned col;

			SourceLoc() : line(0), col(0) {}

			bool operator==(const SourceLoc& b) const {
				return function == b.function && filename == b.filename &&
					line == b.line && col == b.col;
			}

			bool operator!=(const SourceLoc& b) const { return !(*this == b); }
		};

		// In which binary object this trace is located.
		std::string object_filename;

		// The function in the object that contain the trace. This is not the same
		// as source.function which can be an function inlined in object_function.
		std::string object_function;

		// The source location of this trace. It is possible for filename to be
		// empty and for line/col to be invalid (value 0) if this information
		// couldn't be deduced, for example if there is no debug information in the
		// binary object.
		SourceLoc source;

		// An optionals list of "inliners". All the successive sources location
		// from where the source location of the trace (the attribute right above)
		// is inlined. It is especially useful when you compiled with optimization.
		typedef std::vector<SourceLoc> source_locs_t;
		source_locs_t inliners;

		ResolvedTrace() : Trace() {}
		ResolvedTrace(const Trace& mini_trace) : Trace(mini_trace) {}
	};

	/*************** STACK TRACE ***************/

	// default implemention.
	template <typename TAG> class StackTraceImpl {
	public:
		size_t size() const { return 0; }
		Trace operator[](size_t) const { return Trace(); }
		size_t load_here(size_t = 0) { return 0; }
		size_t load_from(void*, size_t = 0, void* = nullptr, void* = nullptr) {
			return 0;
		}
		size_t thread_id() const { return 0; }
		void skip_n_firsts(size_t) {}
	};

	class StackTraceImplBase {
	public:
		StackTraceImplBase()
			: _thread_id(0), _skip(0), _context(nullptr), _error_addr(nullptr) {}

		size_t thread_id() const { return _thread_id; }
		DWORD except_code() const { return _code; }
		const char* except_name() const { return _errn; }

		void skip_n_firsts(size_t n) { _skip = n; }

	protected:
		void load_thread_info() {

		}

		void set_context(void* context) { _context = context; }
		void* context() const { return _context; }

		void set_error_addr(void* error_addr) { _error_addr = error_addr; }
		void* error_addr() const { return _error_addr; }

		size_t skip_n_firsts() const { return _skip; }

	private:
		size_t _thread_id;
		size_t _skip;
		void* _context;
		void* _error_addr;
	protected:
		DWORD _code = -1;
		const char* _errn = nullptr;
	};

	class StackTraceImplHolder : public StackTraceImplBase {
	public:
		size_t size() const {
			return (_stacktrace.size() >= skip_n_firsts())
				? _stacktrace.size() - skip_n_firsts()
				: 0;
		}
		Trace operator[](size_t idx) const {
			if (idx >= size()) {
				return Trace();
			}
			return Trace(_stacktrace[idx + skip_n_firsts()], idx);
		}
		void* const* begin() const {
			if (size()) {
				return &_stacktrace[skip_n_firsts()];
			}
			return nullptr;
		}

	protected:
		std::vector<void*> _stacktrace;
	};

#if defined(BACKWARD_SYSTEM_WINDOWS)

	template <>
	class StackTraceImpl<system_tag::current_tag> : public StackTraceImplHolder {
	public:
		// We have to load the machine type from the image info
		// So we first initialize the resolver, and it tells us this info
		void set_machine_type(DWORD machine_type) { machine_type_ = machine_type; }
		void set_context(CONTEXT* ctx) { ctx_ = ctx; }
		void set_thread_handle(HANDLE handle) { thd_ = handle; }
		void set_error_code(DWORD code) { code_ = code; _code = code; }
		void set_error_name(const char* errn) { errn_ = errn; _errn = errn; }

		NOINLINE void load(DWORD skip, _CONTEXT* ctx, PVOID errorAddr, DWORD code = -1)
		{
			set_context(ctx);
			set_error_addr(errorAddr);
			set_error_code(code);

			uintptr_t stack[64]{};
			RtlCaptureStackBackTrace(skip, 64 + skip, (PVOID*)&stack, nullptr);

			for (uintptr_t addr : stack)
			{
				if (addr == 0)
					break;
				_stacktrace.push_back(reinterpret_cast<void*>(addr));
			}
		}

		NOINLINE size_t load_here(size_t depth = 32, void* context = nullptr, void* error_addr = nullptr)
		{
			set_context(static_cast<CONTEXT*>(context));
			set_error_addr(error_addr);
			CONTEXT localCtx; // used when no context is provided

			if (depth == 0) {
				return 0;
			}

			if (!ctx_) {
				ctx_ = &localCtx;
				RtlCaptureContext(ctx_);
			}

			if (!thd_) {
				thd_ = GetCurrentThread();
			}

			HANDLE process = GetCurrentProcess();

			STACKFRAME64 s;
			memset(&s, 0, sizeof(STACKFRAME64));

			// TODO: 32 bit context capture
			s.AddrStack.Mode = AddrModeFlat;
			s.AddrFrame.Mode = AddrModeFlat;
			s.AddrPC.Mode = AddrModeFlat;
#ifdef _M_X64
			s.AddrPC.Offset = ctx_->Rip;
			s.AddrStack.Offset = ctx_->Rsp;
			s.AddrFrame.Offset = ctx_->Rbp;
#else
			s.AddrPC.Offset = ctx_->Eip;
			s.AddrStack.Offset = ctx_->Esp;
			s.AddrFrame.Offset = ctx_->Ebp;
#endif

			if (!machine_type_) {
#ifdef _M_X64
				machine_type_ = IMAGE_FILE_MACHINE_AMD64;
#else
				machine_type_ = IMAGE_FILE_MACHINE_I386;
#endif
			}

			for (;;) {
				// NOTE: this only works if PDBs are already loaded!
				SetLastError(0);
				if (!StackWalk64(machine_type_, process, thd_, &s, ctx_, NULL,
					SymFunctionTableAccess64, SymGetModuleBase64, NULL))
					break;

				if (s.AddrReturn.Offset == 0)
					break;

				_stacktrace.push_back(reinterpret_cast<void*>(s.AddrPC.Offset));

				if (size() >= depth)
					break;
			}

			return size();
		}

		size_t load_from(void* addr, size_t depth = 32, void* context = nullptr,
			void* error_addr = nullptr) {
			load_here(depth + 8, context, error_addr);

			for (size_t i = 0; i < _stacktrace.size(); ++i) {
				if (_stacktrace[i] == addr) {
					skip_n_firsts(i);
					break;
				}
			}

#undef min
#undef max
			_stacktrace.resize(std::min(_stacktrace.size(), skip_n_firsts() + depth));
			return size();
		}

	private:
		DWORD machine_type_ = 0;
		HANDLE thd_ = 0;
		CONTEXT* ctx_ = nullptr;
		DWORD code_ = 0;
		const char* errn_;
	};

#endif

	class StackTrace : public StackTraceImpl<system_tag::current_tag> {};

	/*************** TRACE RESOLVER ***************/

	class TraceResolverImplBase {
	public:
		virtual ~TraceResolverImplBase() {}

		virtual void load_addresses(void* const* addresses, int address_count)
		{
			(void)addresses;
			(void)address_count;
		}

		template <class ST> void load_stacktrace(ST& st) {
			load_addresses(st.begin(), (int)st.size());
		}

		virtual ResolvedTrace resolve(ResolvedTrace t) { return t; }

	protected:
		std::string demangle(const char* funcname) {
			return _demangler.demangle(funcname);
		}

	private:
		details::demangler _demangler;
	};

	template <typename TAG> class TraceResolverImpl;

#ifdef BACKWARD_SYSTEM_WINDOWS

	// Load all symbol info
	// Based on:
	// https://stackoverflow.com/questions/6205981/windows-c-stack-trace-from-a-running-app/28276227#28276227

	struct module_data {
		std::string image_name;
		std::string module_name;
		void* base_address;
		DWORD load_size;
	};

	class get_mod_info {
		HANDLE process;
		static const int buffer_length = 4096;

	public:
		get_mod_info(HANDLE h) : process(h) {}

		module_data operator()(HMODULE module) {
			module_data ret;
			char temp[buffer_length];
			MODULEINFO mi;

			GetModuleInformation(process, module, &mi, sizeof(mi));
			ret.base_address = mi.lpBaseOfDll;
			ret.load_size = mi.SizeOfImage;

			GetModuleFileNameExA(process, module, temp, sizeof(temp));
			ret.image_name = temp;
			GetModuleBaseNameA(process, module, temp, sizeof(temp));
			ret.module_name = temp;
			std::vector<char> img(ret.image_name.begin(), ret.image_name.end());
			std::vector<char> mod(ret.module_name.begin(), ret.module_name.end());
			SymLoadModule64(process, 0, &img[0], &mod[0], (DWORD64)ret.base_address,
				ret.load_size);
			return ret;
		}
	};

	template <> class TraceResolverImpl<system_tag::windows_tag>
	: public TraceResolverImplBase
	{
	public:
		TraceResolverImpl()
		{
			HANDLE process = GetCurrentProcess();
			CHAR currentDir[MAX_PATH];
			GetCurrentDirectoryA(MAX_PATH, currentDir);

			std::vector<module_data> modules;
			DWORD cbNeeded;
			std::vector<HMODULE> module_handles(1);
			SymInitialize(process, currentDir, false);
			std::ofstream fs("Log.txt", std::ios_base::out | std::ios_base::app);
			fs << "Init: " << std::hex << process << "\n";
			fs.close();
			DWORD symOptions = SymGetOptions();
			symOptions |= SYMOPT_LOAD_LINES | SYMOPT_UNDNAME;
			SymSetOptions(symOptions);
			EnumProcessModules(process, &module_handles[0],
				module_handles.size() * sizeof(HMODULE), &cbNeeded);
			module_handles.resize(cbNeeded / sizeof(HMODULE));
			EnumProcessModules(process, &module_handles[0],
				module_handles.size() * sizeof(HMODULE), &cbNeeded);
			std::transform(module_handles.begin(), module_handles.end(),
				std::back_inserter(modules), get_mod_info(process));
			void* base = modules[0].base_address;
			IMAGE_NT_HEADERS* h = ImageNtHeader(base);
			image_type = h->FileHeader.Machine;
		}

		~TraceResolverImpl() override
		{
			HANDLE process = GetCurrentProcess();
			SymCleanup(process);
		}

		static const int max_sym_len = 255;
		struct symbol_t {
			SYMBOL_INFO sym;
			char buffer[max_sym_len];
		} sym;

		DWORD64 displacement;

		ResolvedTrace resolve(ResolvedTrace t) override {
			HANDLE process = GetCurrentProcess();

			char name[256];

			memset(&sym, 0, sizeof(sym));
			sym.sym.SizeOfStruct = sizeof(SYMBOL_INFO);
			sym.sym.MaxNameLen = max_sym_len;

			if (!SymFromAddr(process, (ULONG64)t.addr, &displacement, &sym.sym)) {
				// TODO:  error handling everywhere
				char* lpMsgBuf;
				DWORD dw = GetLastError();

				if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(char*)&lpMsgBuf, 0, NULL)) {
					std::fprintf(stderr, "%s\n", lpMsgBuf);
					LocalFree(lpMsgBuf);
				}

				// abort();
			}
			UnDecorateSymbolName(sym.sym.Name, (PSTR)name, 256, UNDNAME_COMPLETE);

			DWORD offset = 0;
			IMAGEHLP_LINE line;
			if (SymGetLineFromAddr(process, (ULONG64)t.addr, &offset, &line)) {
				t.object_filename = line.FileName;
				t.source.filename = line.FileName;
				t.source.line = line.LineNumber;
				t.source.col = offset;
			}

			t.source.function = name;
			t.object_filename = "";
			t.object_function = name;

			return t;
		}

		DWORD machine_type() const { return image_type; }

	private:
		DWORD image_type;
	};

#endif

	class TraceResolver : public TraceResolverImpl<system_tag::current_tag> {};

	/*************** CODE SNIPPET ***************/

	class SourceFile {
	public:
		typedef std::vector<std::pair<unsigned, std::string>> lines_t;

		SourceFile() {}
		SourceFile(const std::string& path) {
			// 1. If BACKWARD_CXX_SOURCE_PREFIXES is set then assume it contains
			//    a colon-separated list of path prefixes.  Try prepending each
			//    to the given path until a valid file is found.
			const std::vector<std::string>& prefixes = get_paths_from_env_variable();
			for (size_t i = 0; i < prefixes.size(); ++i) {
				// Double slashes (//) should not be a problem.
				std::string new_path = prefixes[i] + '/' + path;
				_file.reset(new std::ifstream(new_path.c_str()));
				if (is_open())
					break;
			}
			// 2. If no valid file found then fallback to opening the path as-is.
			if (!_file || !is_open()) {
				_file.reset(new std::ifstream(path.c_str()));
			}
		}
		bool is_open() const { return _file->is_open(); }

		lines_t& get_lines(unsigned line_start, unsigned line_count, lines_t& lines) {
			using namespace std;
			// This function make uses of the dumbest algo ever:
			//	1) seek(0)
			//	2) read lines one by one and discard until line_start
			//	3) read line one by one until line_start + line_count
			//
			// If you are getting snippets many time from the same file, it is
			// somewhat a waste of CPU, feel free to benchmark and propose a
			// better solution ;)

			_file->clear();
			_file->seekg(0);
			string line;
			unsigned line_idx;

			for (line_idx = 1; line_idx < line_start; ++line_idx) {
				std::getline(*_file, line);
				if (!*_file) {
					return lines;
				}
			}

			// think of it like a lambda in C++98 ;)
			// but look, I will reuse it two times!
			// What a good boy am I.
			struct isspace {
				bool operator()(char c) { return std::isspace(c); }
			};

			bool started = false;
			for (; line_idx < line_start + line_count; ++line_idx) {
				getline(*_file, line);
				if (!*_file) {
					return lines;
				}
				if (!started) {
					if (std::find_if(line.begin(), line.end(), not_isspace()) == line.end())
						continue;
					started = true;
				}
				lines.push_back(make_pair(line_idx, line));
			}

			lines.erase(
				std::find_if(lines.rbegin(), lines.rend(), not_isempty()).base(),
				lines.end());
			return lines;
		}

		lines_t get_lines(unsigned line_start, unsigned line_count) {
			lines_t lines;
			return get_lines(line_start, line_count, lines);
		}

		// there is no find_if_not in C++98, lets do something crappy to
		// workaround.
		struct not_isspace {
			bool operator()(char c) { return !std::isspace(c); }
		};
		// and define this one here because C++98 is not happy with local defined
		// struct passed to template functions, fuuuu.
		struct not_isempty {
			bool operator()(const lines_t::value_type& p) {
				return !(std::find_if(p.second.begin(), p.second.end(), not_isspace()) ==
					p.second.end());
			}
		};

		void swap(SourceFile& b) { _file.swap(b._file); }

		SourceFile(SourceFile&& from) : _file(nullptr) { swap(from); }
		SourceFile& operator=(SourceFile&& from) {
			swap(from);
			return *this;
		}

	private:
		details::handle<std::ifstream*, details::default_delete<std::ifstream*>>
			_file;

		std::vector<std::string> get_paths_from_env_variable_impl() {
			std::vector<std::string> paths;
			const char* prefixes_str = std::getenv("BACKWARD_CXX_SOURCE_PREFIXES");
			if (prefixes_str && prefixes_str[0]) {
				paths = details::split_source_prefixes(prefixes_str);
			}
			return paths;
		}

		const std::vector<std::string>& get_paths_from_env_variable() {
			static std::vector<std::string> paths = get_paths_from_env_variable_impl();
			return paths;
		}

		SourceFile(const SourceFile&) = delete;
		SourceFile& operator=(const SourceFile&) = delete;
	};

	class SnippetFactory {
	public:
		typedef SourceFile::lines_t lines_t;

		lines_t get_snippet(const std::string& filename, unsigned line_start,
			unsigned context_size) {

			SourceFile& src_file = get_src_file(filename);
			unsigned start = line_start - context_size / 2;
			return src_file.get_lines(start, context_size);
		}

		lines_t get_combined_snippet(const std::string& filename_a, unsigned line_a,
			const std::string& filename_b, unsigned line_b,
			unsigned context_size) {
			SourceFile& src_file_a = get_src_file(filename_a);
			SourceFile& src_file_b = get_src_file(filename_b);

			lines_t lines =
				src_file_a.get_lines(line_a - context_size / 4, context_size / 2);
			src_file_b.get_lines(line_b - context_size / 4, context_size / 2, lines);
			return lines;
		}

		lines_t get_coalesced_snippet(const std::string& filename, unsigned line_a,
			unsigned line_b, unsigned context_size) {
			SourceFile& src_file = get_src_file(filename);

			using std::max;
			using std::min;
			unsigned a = min(line_a, line_b);
			unsigned b = max(line_a, line_b);

			if ((b - a) < (context_size / 3)) {
				return src_file.get_lines((a + b - context_size + 1) / 2, context_size);
			}

			lines_t lines = src_file.get_lines(a - context_size / 4, context_size / 2);
			src_file.get_lines(b - context_size / 4, context_size / 2, lines);
			return lines;
		}

	private:
		typedef details::hashtable<std::string, SourceFile>::type src_files_t;
		src_files_t _src_files;

		SourceFile& get_src_file(const std::string& filename) {
			src_files_t::iterator it = _src_files.find(filename);
			if (it != _src_files.end()) {
				return it->second;
			}
			SourceFile& new_src_file = _src_files[filename];
			new_src_file = SourceFile(filename);
			return new_src_file;
		}
	};

	/*************** PRINTER ***************/

	namespace ColorMode {
		enum type { automatic, never, always };
	}

	class cfile_streambuf : public std::streambuf {
	public:
		cfile_streambuf(FILE* _sink) : sink(_sink) {}
		int_type underflow() override { return traits_type::eof(); }
		int_type overflow(int_type ch) override {
			if (traits_type::not_eof(ch) && fputc(ch, sink) != EOF) {
				return ch;
			}
			return traits_type::eof();
		}

		std::streamsize xsputn(const char_type* s, std::streamsize count) override {
			return static_cast<std::streamsize>(
				fwrite(s, sizeof * s, static_cast<size_t>(count), sink));
		}

	public:
		cfile_streambuf(const cfile_streambuf&) = delete;
		cfile_streambuf& operator=(const cfile_streambuf&) = delete;
	private:
		FILE* sink;
		std::vector<char> buffer;
	};

	namespace Color {
		enum type { yellow = 0, purple = 0, reset = 0 };
	} // namespace Color

	class Colorize {
	public:
		Colorize(std::ostream&) {}
		void activate(ColorMode::type) {}
		void activate(ColorMode::type, FILE*) {}
		void set_color(Color::type) {}
	};

	static const char* GetExceptionName(DWORD code)
	{
		if (code == 0xC0000005L) return "EXCEPTION_ACCESS_VIOLATION";
		if (code == 0x80000002L) return "EXCEPTION_DATATYPE_MISALIGNMENT";
		if (code == 0x80000003L) return "EXCEPTION_BREAKPOINT";
		if (code == 0x80000004L) return "EXCEPTION_SINGLE_STEP";
		if (code == 0xC000008CL) return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		if (code == 0xC000008DL) return "EXCEPTION_FLT_DENORMAL_OPERAND";
		if (code == 0xC000008EL) return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
		if (code == 0xC000008FL) return "EXCEPTION_FLT_INEXACT_RESULT";
		if (code == 0xC0000090L) return "EXCEPTION_FLT_INVALID_OPERATION";
		if (code == 0xC0000091L) return "EXCEPTION_FLT_OVERFLOW";
		if (code == 0xC0000092L) return "EXCEPTION_FLT_STACK_CHECK";
		if (code == 0xC0000093L) return "EXCEPTION_FLT_UNDERFLOW";
		if (code == 0xC0000094L) return "EXCEPTION_INT_DIVIDE_BY_ZERO";
		if (code == 0xC0000095L) return "EXCEPTION_INT_OVERFLOW";
		if (code == 0xC0000096L) return "EXCEPTION_PRIV_INSTRUCTION";
		if (code == 0xC0000006L) return "EXCEPTION_IN_PAGE_ERROR";
		if (code == 0xC000001DL) return "EXCEPTION_ILLEGAL_INSTRUCTION";
		if (code == 0xC0000025L) return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
		if (code == 0xC00000FDL) return "EXCEPTION_STACK_OVERFLOW";
		if (code == 0xC0000026L) return "EXCEPTION_INVALID_DISPOSITION";
		if (code == 0x80000001L) return "EXCEPTION_GUARD_PAGE";
		if (code == 0xC0000008L) return "EXCEPTION_INVALID_HANDLE";
		return "";
	}

	class Printer {
	public:
		bool snippet;
		bool snippet_only_first;
		ColorMode::type color_mode;
		bool address;
		bool object;
		int inliner_context_size;
		int trace_context_size;
		bool exception = true;

		Printer()
			: snippet(true), color_mode(ColorMode::automatic), address(false),
			object(false), inliner_context_size(5), trace_context_size(7) {}

		template <typename ST> FILE* print(ST& st, FILE* fp = stderr) {
			cfile_streambuf obuf(fp);
			std::ostream os(&obuf);
			Colorize colorize(os);
			colorize.activate(color_mode, fp);
			print_stacktrace(st, os, colorize);
			return fp;
		}

		template <typename ST> std::ostream& print(const ST& st, std::ostream& os) {
			Colorize colorize(os);
			colorize.activate(color_mode);
			print_stacktrace(st, os, colorize);
			return os;
		}

		template <typename IT>
		FILE* print(IT begin, IT end, FILE* fp = stderr, size_t thread_id = 0) {
			cfile_streambuf obuf(fp);
			std::ostream os(&obuf);
			Colorize colorize(os);
			colorize.activate(color_mode, fp);
			print_stacktrace(begin, end, os, thread_id, colorize);
			return fp;
		}

		template <typename IT>
		std::ostream& print(IT begin, IT end, std::ostream& os,
			size_t thread_id = 0)
		{
			Colorize colorize(os);
			colorize.activate(color_mode);
			print_stacktrace(begin, end, os, thread_id, colorize);
			return os;
		}

		void print_main_frame(const StackTrace& st, std::ostream& os)
		{
			Trace frame = st[0];

			HMODULE hModule;
			char moduleName[MAX_PATH];
			bool moduleFound = GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				(LPCSTR)frame.addr, &hModule);
			if (moduleFound)
				GetModuleBaseNameA(GetCurrentProcess(), hModule, moduleName, MAX_PATH);
			else
				moduleName[0] = '\0';

			ResolvedTrace resolved = _resolver.resolve(frame);
			if (!resolved.object_function.empty())
				os << moduleName << "!" << resolved.object_function;
			else
				os << moduleName << "!" << std::hex << (uintptr_t)resolved.addr << std::dec;
		}

		TraceResolver const& resolver() const { return _resolver; }

	private:
		TraceResolver _resolver;
		SnippetFactory _snippets;

		template <typename ST>
		void print_stacktrace(ST& st, std::ostream& os, Colorize& colorize)
		{
			print_header(os, st.thread_id(), st.except_code(), st.except_name());
			_resolver.load_stacktrace(st);
			for (size_t trace_idx = st.size(); trace_idx > 0; --trace_idx) {
				print_trace(os, _resolver.resolve(st[trace_idx - 1]), colorize);
			}
		}

		void print_header(std::ostream& os, size_t thread_id, DWORD exceptionCode, const char* exName)
		{
			if (exception)
			{
				if (!exName && exceptionCode != -1)
					exName = GetExceptionName(exceptionCode);

				if (exName)
				{
					os << exName << std::hex;
					if (exceptionCode != -1)
						os << "(0x" << exceptionCode << ")";
					os << std::dec << "\n";
				}
			}
			os << "Stack trace (most recent call last)";
			if (thread_id) {
				os << " in thread " << thread_id;
			}
			os << ":\n";
		}

		void print_trace(std::ostream& os, const ResolvedTrace& trace, Colorize& colorize)
		{
			os << "#" << std::left << std::setw(2) << trace.idx << std::right;
			bool already_indented = true;

			// I don't care so just put here
			HMODULE hModule;
			char moduleName[MAX_PATH];
			bool moduleFound = GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				(LPCSTR)trace.addr, &hModule);
			if (moduleFound)
				GetModuleBaseNameA(GetCurrentProcess(), hModule, moduleName, MAX_PATH);
			else
				moduleName[0] = '\0';

			if (!trace.source.filename.size() || object) {
				os << "   " << trace.object_filename << moduleName
					<< "!" << trace.object_function << " at " << trace.addr << "\n";
				already_indented = false;
			}

			for (size_t inliner_idx = trace.inliners.size(); inliner_idx > 0;
				--inliner_idx) {
				if (!already_indented) {
					os << "   ";
				}
				const ResolvedTrace::SourceLoc& inliner_loc =
					trace.inliners[inliner_idx - 1];
				print_source_loc(os, " | ", inliner_loc);
				if (snippet || (snippet_only_first && trace.idx == 0)) {
					print_snippet(os, "    | ", inliner_loc, colorize, Color::purple,
						inliner_context_size);
				}
				already_indented = false;
			}

			if (trace.source.filename.size()) {
				if (!already_indented) {
					os << "   ";
				}
				print_source_loc(os, "   ", trace.source, trace.addr);
				if (snippet || (snippet_only_first && trace.idx == 0)) {
					print_snippet(os, "      ", trace.source, colorize, Color::yellow,
						trace_context_size);
				}
			}
		}

		void print_snippet(std::ostream& os, const char* indent,
			const ResolvedTrace::SourceLoc& source_loc,
			Colorize& colorize, Color::type color_code,
			int context_size) {
			using namespace std;
			typedef SnippetFactory::lines_t lines_t;

			lines_t lines = _snippets.get_snippet(source_loc.filename, source_loc.line,
				static_cast<unsigned>(context_size));

			for (lines_t::const_iterator it = lines.begin(); it != lines.end(); ++it) {
				if (it->first == source_loc.line) {
					colorize.set_color(color_code);
					os << indent << ">";
				}
				else {
					os << indent << " ";
				}
				os << std::setw(4) << it->first << ": " << it->second << "\n";
				if (it->first == source_loc.line) {
					colorize.set_color(Color::reset);
				}
			}
		}

		void print_source_loc(std::ostream& os, const char* indent,
			const ResolvedTrace::SourceLoc& source_loc,
			void* addr = nullptr)
		{
			// I don't care so just put here
			HMODULE hModule;
			char moduleName[MAX_PATH];
			bool moduleFound = GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				(LPCSTR)addr, &hModule);
			if (moduleFound)
				GetModuleBaseNameA(GetCurrentProcess(), hModule, moduleName, MAX_PATH);
			else
				moduleName[0] = '\0';

			os << indent << moduleName << "!" << source_loc.function << " in " << source_loc.filename << "\", line "
				<< source_loc.line;

			if (address && addr != nullptr) {
				os << " [" << addr << "]";
			}
			os << "\n";
		}
	};

	/*************** SIGNALS HANDLING ***************/

#ifdef BACKWARD_SYSTEM_WINDOWS

	class SignalHandling
	{
		LPTOP_LEVEL_EXCEPTION_FILTER m_PrevFilter;
		_crt_signal_t m_PrevSignal;
		terminate_handler m_PrevTerminate;
		_purecall_handler m_PrevPurecall;
		_invalid_parameter_handler m_InvalidParam;
	public:
		SignalHandling(const std::vector<int> & = std::vector<int>())
		{
			m_PrevFilter = SetUnhandledExceptionFilter(crash_handler);
			m_PrevSignal = signal(SIGABRT, signal_handler);
			m_PrevTerminate = std::set_terminate(&terminator);
			m_PrevPurecall = _set_purecall_handler(&purecall);
			m_InvalidParam = _set_invalid_parameter_handler(&invalid_parameter_handler);

			_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
		}

		~SignalHandling()
		{
			SetUnhandledExceptionFilter(m_PrevFilter);
			signal(SIGABRT, m_PrevSignal);
			std::set_terminate(m_PrevTerminate);
			_set_purecall_handler(m_PrevPurecall);
			_set_invalid_parameter_handler(m_InvalidParam);
		}

	private:
		static CONTEXT* ctx() {
			static CONTEXT data;
			return &data;
		}

		enum class crash_status { running, crashed, normal_exit, ending };

		static crash_status& crashed() {
			static crash_status data;
			return data;
		}

		static std::mutex& mtx() {
			static std::mutex data;
			return data;
		}

		static std::condition_variable& cv() {
			static std::condition_variable data;
			return data;
		}

		static HANDLE& thread_handle() {
			static HANDLE handle;
			return handle;
		}

		//std::thread reporter_thread_;

		// TODO: how not to hardcode these?
		static const constexpr int signal_skip_recs =
#ifdef __clang__
			// With clang, RtlCaptureContext also captures the stack frame of the
			// current function Below that, there ar 3 internal Windows functions
			4
#else
			// With MSVC cl, RtlCaptureContext misses the stack frame of the current
			// function The first entries during StackWalk are the 3 internal Windows
			// functions
			3
#endif
			;

		static int& skip_recs() {
			static int data;
			return data;
		}

		static inline void terminator() {
			HandleStacktrace(signal_skip_recs, nullptr, "STD::TERMINATE");
			abort();
		}

		static inline void purecall() {
			HandleStacktrace(signal_skip_recs, nullptr, "PURECALL");
			abort();
		}

		static inline void signal_handler(int) {
			HandleStacktrace(signal_skip_recs, nullptr, "SIGABRT");
			abort();
		}

		static inline void __cdecl invalid_parameter_handler(const wchar_t*,
			const wchar_t*,
			const wchar_t*,
			unsigned int,
			uintptr_t) {
			HandleStacktrace(signal_skip_recs, nullptr, "INVALID_PARAMETER");
			abort();
		}

		static inline std::mutex m_BreakMutex;
		static inline LPVOID m_DebugBreakAddress = nullptr;
		static inline std::string m_DebugBreakFrame;
		static inline ULONGLONG m_DebugBreakLastTime;
	public:
		static bool GetDebugBreak(const char** ppFrameName)
		{
			m_BreakMutex.lock();
			*ppFrameName = m_DebugBreakFrame.c_str();
			bool active = GetTickCount64() - m_DebugBreakLastTime < 1000;
			m_BreakMutex.unlock();
			return active;
		}

		NOINLINE static LONG WINAPI HandleDebugBreak(const EXCEPTION_POINTERS* info, int skip = 0)
		{
			m_BreakMutex.lock();

			m_DebugBreakLastTime = GetTickCount64();
			if (m_DebugBreakAddress != info->ExceptionRecord->ExceptionAddress)
			{
				m_DebugBreakAddress = info->ExceptionRecord->ExceptionAddress;

				std::stringstream ss;
				StackTrace stack;
				Printer printer;

				stack.load(9 + skip, nullptr, nullptr);
				printer.print_main_frame(stack, ss);
				m_DebugBreakFrame = ss.str();
			}

			m_BreakMutex.unlock();

			Yield();
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	private:

		NOINLINE static LONG WINAPI crash_handler(EXCEPTION_POINTERS* info)
		{
			// For debug breaks, just wait until debugger attaches
			if (info->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
				return HandleDebugBreak(info);

			HandleStacktrace(3, info);
			return EXCEPTION_CONTINUE_SEARCH;
		}

		static void HandleStacktrace(int skip = 0, const _EXCEPTION_POINTERS* exInfo = nullptr, const char* exName = nullptr);
	};

#endif // BACKWARD_SYSTEM_WINDOWS

} // namespace backward

#endif /* H_GUARD */

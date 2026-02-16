// engine.hpp

#pragma once

#include <unordered_map>
#include "object.hpp"
#include "input_window.hpp"
#include "types.hpp"

namespace triton
{
	struct sCapabilities;
	class cContext;
	class iApplication;
	template <typename T>
	class cStack;

	class cEngine final : public iObject
	{
		TRITON_OBJECT(cEngine)

	public:
		explicit cEngine(cContext* context, iApplication* app);
		virtual ~cEngine() override final;

		void Initialize();
		void Shutdown();
		void Run();

		inline iApplication* GetApplication() const { return _app; }
		inline const sCapabilities* GetCapabilities() const { return _caps; }

	private:
		iApplication* _app = nullptr;
		const sCapabilities* _caps = nullptr;
	};
}
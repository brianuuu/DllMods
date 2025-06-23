#pragma once

namespace Sonic
{
	class CShapeCollision
	{
	public:
		static constexpr size_t ms_pVtbl = 0x16A191C;
	
		CShapeCollision() = default;
		virtual ~CShapeCollision() {}

		BB_INSERT_PADDING(0x48) {};
		void* cachingShapePhantom{}; // T = hkpCachingShapePhantom*
		BB_INSERT_PADDING(0x10) {};
	};
	
	BB_ASSERT_OFFSETOF(CShapeCollision, cachingShapePhantom, 0x4C);
	BB_ASSERT_SIZEOF(CShapeCollision, 0x60);
}
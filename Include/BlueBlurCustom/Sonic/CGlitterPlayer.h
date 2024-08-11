namespace Sonic
{
	class CParticleController;

	class CGlitterPlayer : public Hedgehog::Base::CObject
	{
	public:
		virtual ~CGlitterPlayer();

	private:
		virtual void PlayOneshotByMatrix(const Hedgehog::Math::CMatrix& in_Matrix, const Hedgehog::Base::CSharedString& in_AssetName, float in_Size, int in_ID) {}
		virtual void PlayOneshotByNode(const boost::shared_ptr<Hedgehog::Mirage::CMatrixNode>& in_MatrixNode, const Hedgehog::Base::CSharedString& in_AssetName, float in_Size, int in_ID) {}

		virtual bool PlayContinuousByMatrix(Hedgehog::Base::THolder<CGameDocument> in_Holder,
			const Hedgehog::Math::CMatrix& in_Matrix,
			const Hedgehog::Base::CSharedString& in_AssetName, float in_Size, int usuallyOne,
			int usuallyZero)
		{
			return false;
		}
		virtual bool PlayContinuousByNode(Hedgehog::Base::THolder<CGameDocument> in_Holder,
			const boost::shared_ptr<Hedgehog::Mirage::CMatrixNode>& in_spNode,
			const Hedgehog::Base::CSharedString& in_AssetName, float in_Size, int usuallyOne,
			int usuallyZero)
		{
			return false;
		}

	public:

		// I prefer function overloads personally, so I'm doing it this way.
		void PlayOneshot(const Hedgehog::Math::CMatrix& in_Matrix, const Hedgehog::Base::CSharedString& in_AssetName, float in_Size, int in_ID)
		{
			PlayOneshotByMatrix(in_Matrix, in_AssetName, in_Size, in_ID);
		}
		void PlayOneshot(const boost::shared_ptr<Hedgehog::Mirage::CMatrixNode>& in_MatrixNode, const Hedgehog::Base::CSharedString& in_AssetName, float in_Size, int in_ID)
		{
			PlayOneshotByNode(in_MatrixNode, in_AssetName, in_Size, in_ID);
		}
		bool PlayContinuous(const Hedgehog::Base::TSynchronizedPtr<Sonic::CGameDocument>& pGameDocument,
			const boost::shared_ptr<Hedgehog::Mirage::CMatrixNode>& in_spNode,
			const Hedgehog::Base::CSharedString& in_AssetName, float in_Size, int usuallyOne = 1, int usuallyZero = 0)
		{
			return PlayContinuousByNode(pGameDocument.get(), in_spNode, in_AssetName, in_Size, usuallyOne, usuallyZero);
		}
		bool PlayContinuous(const Hedgehog::Base::TSynchronizedPtr<Sonic::CGameDocument>& pGameDocument,
			const Hedgehog::Math::CMatrix& in_Matrix,
			const Hedgehog::Base::CSharedString& in_AssetName, float in_Size, int usuallyOne = 1, int usuallyZero = 0)
		{
			return PlayContinuousByMatrix(pGameDocument.get(), in_Matrix, in_AssetName, in_Size, usuallyOne, usuallyZero);
		}

		CParticleController* m_pParticleController;

		static CGlitterPlayer* Make(CGameDocument* pGameDocument)
		{
			BB_FUNCTION_PTR(CGlitterPlayer*, __cdecl, Func, 0x01255B40, Sonic::CGameDocument * in_pGameDocument);
			CGlitterPlayer* result = Func(pGameDocument);

			return result;
		}
	};
}
#ifndef engine_particle_factoryH
#define engine_particle_factoryH




#include "timing.h"
#include "aspect.h"
#include "renderer/renderer.h"
#include "../container/vectorList.h"

namespace Engine
{

	struct TParticle
	{
		M::TVec3<>				position;
		float				size;
	};

	struct TColoredParticle:public TParticle
	{
		M::TVec4<>				color[4];

		TColoredParticle()	{
								color[0] = color[1] = color[2] = color[3] = Vector4<>::one;
	                        }
		void				setColor(float red, float green, float blue, float alpha)
	                        {
	                            M::Vec::def(color[0],red,green,blue,alpha);
	                            M::Vec::def(color[1],red,green,blue,alpha);
	                            M::Vec::def(color[2],red,green,blue,alpha);
	                            M::Vec::def(color[3],red,green,blue,alpha);
	                        }
	};



	template <class GL, typename Particle = TParticle>
		class Particles:public PreAllocVector<Particle>
		{
		private:
			typename GL::Material		material;

			void						setup();

		protected:
			GL							*renderer;					//!< Pointer to the used rendering context (not NULL)

		public:
			typedef PreAllocVector<Particle>	Super;
			
			Particles()					{setup();};
		template <class Float>
		    void						render(const Aspect<Float>&aspect);
		    void						renderAsLines(const M::TVec3<>&delta0, const M::TVec3<>&delta1);
		    Particle&					add(const M::TVec3<>&position, float size=1);
		    void						translate(const M::TVec3<>&delta);
		    void						translate(float dx, float dy, float dz);
			void						setTexture(const typename GL::Texture::Reference&texture);
			void						setTexture(const typename GL::Texture&texture)	{setTexture(texture.reference());}

			void						setRenderer(GL*renderer);			//!< Sets the renderer to use for subsequent geometry/texture uploads and rendering.
			void						setRenderer(GL&renderer);			//!< Sets the renderer to use for subsequent geometry/texture uploads and rendering.
			GL*							getRenderer()	const;				//!< Retrieves the currently used renderer

		};

	template <class GL, typename Particle = TColoredParticle>
		class ColoredParticles:public Particles<GL,Particle>
		{
		public:
			typedef Particles<GL,Particle>		Super;
		template <class Float>
	        void						render(const Aspect<Float>&aspect);
		template <class Float>
		    void						render(const Aspect<Float>&aspect, const M::TVec3<>&color_override);
		};

}

#include "particle_factory.tpl.h"

#endif

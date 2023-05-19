#ifndef flareH
#define flareH


#include <engine/renderer/opengl.h>
#include <engine/display.h>
#include <image/img_generators.h>
#include <general/random.h>

using namespace Engine;

extern Display<OpenGL>				display;	


/**
	@brief Semi-automated flare renderer
*/
namespace Flare
{
	struct TFlareSprite	//! Flare sprite along the flare sprite line
	{
		const OpenGL::Texture		*texture;	//!< Texture to use for this flare sprite. Empty sprites are not rendered
		OpenGL::FloatType			width,		//!< Width of this sprite on the screen (measured in height units: 1=full screen height)
									height,		//!< Height of this sprite on the screen (1= full screen height)
									intensity,	//!< Sprite intensity (0-1) multiplied by the respective flare intensity
									position;	//!< Relative position along the flare line (0=light source, 1=screen center). Values may lie outside the [0,1] range
		bool						primary,	//!< Attribute set for sprites directly relating to the light source (i.e. at position 0)
									sensitive;	//!< True if this light is sensitive to and dimmed by ambient light
		
		M::TVec3<OpenGL::FloatType>	color;	//!< Color modifier
	};


	/**
		@brief Flare structure holding additional information of an individual light source
	*/
	class LightInfo
	{
	public:
			float			scale,
							visibility,		//!< Current visibility based on occlusion
							intensity,		//!< Current intensity based on direction (if directional)
							distance;		//!< Distance between the camera and this flare point
			M::TVec2<>			projected;		//!< Last projected screen coordinates
			bool			clipped,		//!< True if the light currently resides outside the screen clipping area
							was_clipped,	//!< True if the light was outside the screen clipping area before the last map() call. Reset (to false) by render() in case it is now visible
							occluded,		//!< Specifies whether or not this light is occluded by some object in the viewport. Valid only if @a clipped is false
							tagged;			//!< Custom tag. Used to match the currently known lights with the lights of the active light scenario
			Engine::PLight	reference;		//!< Pointer to the actual light structure
		
			/**/			LightInfo():scale(100),visibility(1),intensity(1),distance(1),clipped(false),occluded(false),tagged(false),reference(NULL)	{}
			/**/			LightInfo(Engine::PLight reference_):scale(100),visibility(1),intensity(1),distance(1),clipped(false),occluded(false),tagged(false),reference(reference_)	{}

			float			GetSize()	const	{return scale * reference->GetSize();}
	};



	/**
	@brief Notifies the flare renderer that the composition of light sources has changed
		
	Unless this method is invoked the flare renderer assumes the light scenery has not changed.
	*/
	void			SignalLightsChanged();
	
	

	/**
	@brief Maps light sources to a camera and determine's all lights' visibility based on the current depth buffer content
		
	The method checks for both frustum clipping and depth buffer occlusion (unless @a check_occlusion is false).<br>
	Must be invoked every frame before calling render().
		
	@param camera Camera to retrieve the frustum and depth range of. The matrices are assumed to be already applied
	@param check_occlusion Set true to check for occlusion based on the current frame buffer content
	@param isOccluded Function pointer to retrieve occlusion by other means than depth buffer check. The return value will be applied as initial occlusion. Occluded light sources will not be checked for depth buffer visibility if @a check_occlusion is not false
	*/
	void			Map(const Camera<float>&camera, bool check_occlusion=true, bool (*isOccluded)(const Engine::LightData*)=NULL);
	
	/**
	@brief Re-checks every visible light for occlusion based on the current depth buffer content.
		
	This method should be invoked inbetween map() and render() whenever additional objects were rendered to the frame buffer since the last map() or recheckOcclusion() call.
	The specified camera object is only to determine the applicable depth range. Additional frustum clipping tests are not performed.
		
	@param camera Camera object to retrieve the current depth range of. No matrix loading or frustum testing is performed.
	*/
	void			RecheckOcclusion(const Camera<float>&camera);
	
	/**
		@brief Renders flare sprites to the frame buffer based on the currently known light colors, positions, and visibilities.
		
		Sprite textures are generated the first time render() is invoked. map() must be called each frame before invoking this method.
		The position and intensity of flare sprites is determined automatically based on the current camera view direction. Rendering is performed using an additive blend function.
		
		@param ambient_light Ambient light intensity (in the range [0, 1]) used to dim flare sprite intensity.
		@param time_delta Time (in seconds) that passed since the last frame. This value is used to blend light visibility
	*/
	void			Render(float ambient_light, float time_delta);

	/**
		@brief Re-renders all sprites based on their current state
		
		No opacity checks are performed, simply the lights re-projected and rendered based on their current configuration.
		This function is best used for specialized screen capture algorithms that focus on certain image section without actually changing the view.
		
		@param camera Camera to use for projection
		@param screen_center_x Screen center X coordinate in camera projected space. 0 for a centered camera
		@param screen_center_y Screen center Y coordinate in camera projected space. 0 for a centered camera
		@param screen_scale Relatve size of the visible portion. 1 for a standard camera, Less for zooming cameras
		@param ambient_light Ambient light intensity (in the range [0, 1]) used to dim flare sprite intensity.
	*/
	void			Rerender(const Camera<float>&camera, float screen_center_x, float screen_center_y, float screen_scale, float ambient_light);

	struct FlareCenter
	{
		M::TVec3<>		coordinates;
		float		size;
		M::TVec3<>		color;
	};

	void			SetFlareSequence(const FlareCenter*centers, count_t numCenters);
	inline void		SetFlareSequence(const ArrayData<FlareCenter>&centers)	{SetFlareSequence(centers.pointer(),centers.Count());}
	inline void		SetFlareSequence(const BasicVector<FlareCenter>&centers){SetFlareSequence(centers.pointer(),centers.Count());}
}


#endif

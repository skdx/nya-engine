//https://code.google.com/p/nya-engine/

#ifndef __APPLE__

#include "platform_specific_gl.h"
#include "render.h"
#include <string>

namespace render
{

void *get_exact_extension(const char*ext_name)
{
	#ifdef _WIN32
		return (void*)wglGetProcAddress(ext_name);
	#else
		return (void*)glXGetProcAddressARB((const GLubyte *)ext_name);	
	#endif
}

void *get_extension(const char*ext_name)
{
	if(!ext)
	{
		get_log()<<log::error_internal<<"invalid extension name\n";
		return 0;
	}

	void *ext = get_exact_extension(ext_name);
	if(ext)
		return ext;

	const static std::string arb("ARB");
	const std::string ext_name_arb = std::string(ext_name)+arb;
	ext = get_exact_extension(ext_name_arb.c_str());
	if(ext)
		return ext;

	const static std::string ext("EXT");
	const std::string ext_name_ext = std::string(ext_name)+ext;
	ext = get_exact_extension(ext_name_ext.c_str());
	if(!ext)
		get_log()<<log::error<<"unable to initialise extension "<<ext_name<<"\n";
		
	return ext;
}

}

#endif


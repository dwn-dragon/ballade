##	Libs subdirectory
#	include here the libraries

#	External misc headers
add_subdirectory(external)

#	Graphics libraries
add_subdirectory(glfw)
add_subdirectory(glbinding)
add_subdirectory(glm)

#	CLI parser
add_subdirectory(CLI11)

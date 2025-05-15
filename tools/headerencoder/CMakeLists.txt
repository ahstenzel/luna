add_executable(headerencoder main.cpp)
target_include_directories(headerencoder PRIVATE
	"${PROJECT_SOURCE_DIR}/vendor/"
	"${PROJECT_SOURCE_DIR}/vendor/base64/include"
	"${PROJECT_SOURCE_DIR}/vendor/json/include"
)
target_link_libraries(headerencoder libcppvex base64)
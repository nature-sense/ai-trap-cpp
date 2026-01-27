project('ai-trap-cpp', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++23' ])
                     #'werror = false'  ])

tomlplusplus_proj = subproject('tomlplusplus')

cc = meson.get_compiler('cpp')

if cc.has_argument('-Wno-deprecated-enum-enum-conversion')
    add_project_arguments('-Wno-deprecated-enum-enum-conversion', language: 'cpp')
endif

lib_camera_dep  = cc.find_library('camera', required: true,  dirs: ['/usr/local/lib/aarch64-linux-gnu'])
lib_camera_base_dep  = cc.find_library('camera-base', required: true,  dirs: ['/usr/local/lib/aarch64-linux-gnu'])
lib_pisp_dep  = cc.find_library('pisp', required: true, dirs: ['/usr/local/lib/aarch64-linux-gnu'])
lib_ncnn_dep = cc.find_library('ncnn', required: true, dirs: ['/usr/local/lib/ncnn'])

openmp_dep = dependency('openmp', required: true)
boost_deps = dependency('boost', modules: ['filesystem'], required: true)
opencv_deps = dependency('opencv4', modules: ['core'], required: true)
rocksdb_dep = dependency('rocksdb', required: true)
protobuf_dep = dependency('protobuf', required: true)
tomlplusplus_dep = dependency('tomlplusplus')

#protoc = find_program('protoc', required: true)

inc_dirs = include_directories(
     #'generated',
     'src',
     '/usr/local/include/libcamera',
     '/usr/local/include/libpisp'
    )

exe = executable('ai-trap-cpp',
                 'src/main.cpp',
                 'src/detection_camera/picamera3/picamera3_actor.cpp',
                 'src/detection_model/yolo11_ncnn/yolo11_ncnn_actor.cpp',
                 'src/references/capture_reference/image_reference.cpp',

                 #'src/websocket_server/listener.cpp',
                 #'src/websocket_server/session.cpp',
                 #'src/websocket_server/websocket_server.cpp',
                 #'src/session_database/session_database.cpp',

                 #'generated/protocol.pb.cc',
                 #'generated/sessions.pb.cc',
                 install : true,
                 dependencies: [
                     lib_camera_dep,
                     lib_camera_base_dep,
                     lib_pisp_dep,
                     boost_deps,
                     opencv_deps,
                     lib_ncnn_dep,
                     openmp_dep,
                     tomlplusplus_dep
                     #rocksdb_dep,
                     #protobuf_dep,
                 ],
                 include_directories : inc_dirs,
)

#test('basic', exe)

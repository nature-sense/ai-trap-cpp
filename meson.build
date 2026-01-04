project('ai-trap-cpp', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++23'])

cc = meson.get_compiler('cpp')
#lib_camera_dep  = cc.find_library('camera', required: true,  dirs: ['/usr/local/lib/aarch64-linux-gnu'])
#lib_camera_base_dep  = cc.find_library('camera-base', required: true,  dirs: ['/usr/local/lib/aarch64-linux-gnu'])
#lib_pisp_dep  = cc.find_library('pisp', required: true, dirs: ['/usr/local/lib/aarch64-linux-gnu'])
lib_ncnn_dep  = dependency('ncnn')
#cc.find_library('ncnn', required: true) # dirs: ['/usr/local/lib/ncnn'])
openmp_dep = dependency('openmp')
boost_deps = dependency('boost', modules: ['filesystem'])
opencv_deps = dependency('opencv4', modules: ['core'])
rocksdb_dep = dependency('rocksdb')

protoc = find_program('protoc', required: true)
protobuf_dep = dependency('protobuf', required: true)
proto_srcs = ['proto/protocol.proto']

proto_gen = generator(
    protoc,
    output: ['@BASENAME@.pb.cpp', '@BASENAME@.pb.h'],
    arguments: [
        '--proto_path=@CURRENT_SOURCE_DIR@',
        '--cpp_out=@BUILD_DIR@',
        '@INPUT@'
    ]
)

generated_sources = []
foreach source : proto_srcs
    generated_sources += proto_gen.process(source)
endforeach

inc_dirs = include_directories(
    ['include',
     #'/usr/local/include/libcamera',
     #'/usr/local/include/libpisp'
    ])



exe = executable('ai-trap-cpp',
                 'src/main.cpp',
                 #'src/detection_camera.cpp',
                 #'src/yolo11_ncnn_model.cpp',
                 #'src/image_reference.cpp',
                 'src/websocket/listener.cpp',
                 'src/websocket/session.cpp',
                 'src/websocket_server.cpp',
                 'src/session_database.cpp',
                 install : true,
                 dependencies: [
                     #lib_camera_dep,
                     #lib_camera_base_dep,
                     #lib_pisp_dep,
                     boost_deps,
                     opencv_deps,
                     lib_ncnn_dep,
                     openmp_dep,
                     rocksdb_dep
                 ],
                 include_directories : inc_dirs,

)

#test('basic', exe)

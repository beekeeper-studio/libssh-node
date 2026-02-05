{
  "targets": [
    {
      "target_name": "libssh_node",
      "sources": [
        "src/binding.cc",
        "src/ssh_session.cc",
        "src/ssh_channel.cc",
        "src/ssh_sftp.cc",
        "src/async_workers.cc",
        "src/utils.cc"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "defines": [
        "NAPI_VERSION=8",
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "cflags_cc": ["-std=c++17"],
      "conditions": [
        [
          "OS=='linux'",
          {
            "libraries": ["-lssh"],
            "cflags_cc": ["-fexceptions"]
          }
        ],
        [
          "OS=='mac'",
          {
            "libraries": ["-lssh"],
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "CLANG_CXX_LIBRARY": "libc++",
              "MACOSX_DEPLOYMENT_TARGET": "10.15",
              "OTHER_CPLUSPLUSFLAGS": ["-std=c++17", "-stdlib=libc++"]
            }
          }
        ],
        [
          "OS=='win'",
          {
            "libraries": ["ssh.lib"],
            "msvs_settings": {
              "VCCLCompilerTool": {
                "ExceptionHandling": 1,
                "AdditionalOptions": ["/std:c++17"]
              }
            }
          }
        ]
      ]
    }
  ]
}

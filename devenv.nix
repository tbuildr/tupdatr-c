{
  pkgs,
  config,
  ...
}: {
  # Provides the C compiler toolchain, Make, pkg-config,
  # GDB and Valgrind.
  languages.c = {
    enable = true;

    # Uses clang-tools/clangd rather than ccls.
    lsp.enable = false;
  };

  packages = with pkgs; [
    # GNU Binary Utilies: ld, as, ar, objdump, nm, strip, readelf / size
    binutils

    # Pretty menuing
    ncurses
    # Debugging and tracing.
    strace
    ltrace
    # Static analysis and compilation database generation.
    clang-tools
    cppcheck
    bear
    # Build acceleration.
    ccache
    # Build systems.
    cmake
    ninja
    meson

    # Documentation (supports C, Objective-C, C++, PHP, Java, Python etc.)
    doxygen
    graphviz
  ];

  env = {
    CC = "gcc";
    CXX = "g++";
    CCACHE_DIR = "${config.devenv.root}/.ccache";
  };

  enterShell = ''
    mkdir -p "$CCACHE_DIR"
  '';
}

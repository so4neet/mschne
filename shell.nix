{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
    buildInputs = with pkgs; [
        sdl3
        assimp
        cglm
        glslang
        sdl3-image
        shaderc
    ];
}

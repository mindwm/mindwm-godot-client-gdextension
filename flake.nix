{
  description = "Godot extention to comminucate with X11 server";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/24.05";
    flake-parts.url = "github:hercules-ci/flake-parts";
    godot-cpp.url = "github:omgbebebe/godot-cpp";
  };

  outputs = inputs@{ flake-parts, godot-cpp, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [];
      systems = [ "x86_64-linux" "aarch64-linux" ];
      perSystem = { config, self', inputs', pkgs, system, ... }:
        let
          godot-cpp-release = godot-cpp.packages.${system}.template_release;
          godot-cpp-debug = godot-cpp.packages.${system}.template_debug;
        in { 
          packages = rec {
          release = pkgs.callPackage ./package.nix { godot-cpp = godot-cpp-release; withTarget = "template_release"; };
          debug = pkgs.callPackage ./package.nix { godot-cpp = godot-cpp-debug; withTarget = "template_debug"; };
          default = release;
        };
        devShells.default = config.packages.debug;
      };
      flake = {
      };
    };
}

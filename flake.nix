{
  description = "Dev envrinment for Godot";

	inputs.nixpkgs.url = "github:nixos/nixpkgs/23.11";
	inputs.unstable.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";
#  inputs.flake-utils.follows = "nixpkgs";
  inputs.godot-cpp.url = "github:omgbebebe/godot-cpp";
#  inputs.godot-cpp.follows = "nixpkgs";

  outputs = { self, nixpkgs, unstable, flake-utils, godot-cpp }:
    flake-utils.lib.eachDefaultSystem
      (system:
         let mkPkgs = pkgs: extraOverlays: import pkgs {
               inherit system;
               config.allowUnfree = true; # forgive me Stallman senpai
             };
             pkgs = mkPkgs nixpkgs [];
             godot-cpp-headers = godot-cpp.packages.${system}.template_debug;
        in
        {
          devShell = import ./shell.nix { inherit pkgs; godot-cpp = godot-cpp-headers; };
        }
      );
}

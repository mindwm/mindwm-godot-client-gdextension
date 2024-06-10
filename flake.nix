{
  description = "Dev envrinment for Godot";

	inputs.nixpkgs.url = "github:nixos/nixpkgs/23.11";
	inputs.unstable.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, unstable, flake-utils }:
    flake-utils.lib.eachDefaultSystem
      (system:
         let mkPkgs = pkgs: extraOverlays: import pkgs {
               inherit system;
               config.allowUnfree = true; # forgive me Stallman senpai
             };
             pkgs = mkPkgs nixpkgs [];
        in
        {
          devShell = import ./shell.nix { inherit pkgs; };
        }
      );
}

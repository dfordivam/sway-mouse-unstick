{
  description = "Reset sway's stuck pressed-button count via a wlr virtual pointer";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = { self, nixpkgs }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" ];
      forAll = f: nixpkgs.lib.genAttrs systems (s: f nixpkgs.legacyPackages.${s});
    in {
      packages = forAll (pkgs: {
        default = pkgs.stdenv.mkDerivation {
          pname = "sway-unstick";
          version = "0.1";
          src = ./.;
          strictDeps = true;
          nativeBuildInputs = [ pkgs.pkg-config pkgs.wayland-scanner ];
          buildInputs = [ pkgs.wayland pkgs.wlr-protocols ];
          makeFlags = [ "PREFIX=${placeholder "out"}" ];
          meta.mainProgram = "sway-unstick";
        };
      });

      devShells = forAll (pkgs: {
        default = pkgs.mkShell { inputsFrom = [ self.packages.${pkgs.system}.default ]; };
      });
    };
}

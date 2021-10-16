let
  sources = import ./nix/sources.nix;
  nixpkgs = sources.nixpkgs;
  pkgs = import nixpkgs {};
  armPkgs = import nixpkgs {
    crossSystem = {
      config = "arm-none-eabi";
    };
  };
in

armPkgs.mkShell {
  buildInputs = [ ]; # your dependencies here
  nativeBuildInputs = with pkgs; [ mgba gdb re2c clang valgrind ];
}

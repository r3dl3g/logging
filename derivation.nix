# https://discourse.nixos.org/t/how-to-use-a-nix-derivation-from-a-local-folder/5498/4

{ stdenv, lib, pkgs } :

stdenv.mkDerivation {
  pname = "logging";
  version = "1.1.2";

  src = ./.;

  nativeBuildInputs = with pkgs; [ cmake ];

  enableParallelBuilding = true;

  outputs = [ "out" "dev" ];

  cmakeFlags = [
    "-DLOGGING_CONFIG_INSTALL=ON"
    "-DLOGGING_BUILD_STATIC_MODULE_LIB=OFF"
  ];

  meta = with lib; {
    description = "A c++ logging library";
    homepage = "https://github.com/r3dl3g/logging";
    changelog = "https://github.com/r3dl3g/logging/releases/tag/v${version}";
    license = licenses.mit;
    maintainers = with maintainers; [ r3dl3g ];
    platforms = with platforms; linux;
  };
}

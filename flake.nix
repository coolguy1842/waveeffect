{
    description = "Wave effect for keychron keyboard";
    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    };

    outputs = { nixpkgs, systems, ... } @ inputs: let
        forEachSystem = nixpkgs.lib.genAttrs (import systems);
    in {
        devShells = forEachSystem (system: let
            pkgs = nixpkgs.legacyPackages.${system};
        in {
            default = pkgs.mkShell {
                hardeningDisable = [ "all" ];

                nativeBuildInputs = with pkgs; [
                    clang
                    pkgconf
                    ninja
                    cmake
                    meson
                ];

                buildInputs = with pkgs; [
                    hidapi
                    libvirt
                    libevdev
                ];
            };
        });

        packages = forEachSystem (system: let
            pkgs = nixpkgs.legacyPackages.${system};
        in {
            default = pkgs.stdenv.mkDerivation {
                name = "waveeffect";
                version = "1.0";
                src = ./.;

                nativeBuildInputs = with pkgs; [
                    clang
                    pkgconf
                    ninja
                    cmake
                    meson
                ];

                buildInputs = with pkgs; [
                    hidapi
                    libvirt
                    libevdev
                ];

                configurePhase = ''
                    meson setup -Dprefix=$out build
                '';

                buildPhase = ''
                    meson compile -C build
                '';

                installPhase = ''
                    mkdir -p $out/bin
                    cp build/waveeffect $out/bin/
                '';

                meta = with pkgs.lib; {
                    description = "Wave effect that works on a Keychron V6";
                    homepage = "https://github.com/coolguy1842/waveeffect/";
                    license = licenses.gpl3;
                };
            };
        });
    };
}
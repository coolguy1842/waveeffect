{
    description = "Wave effect for keychron keyboard";
    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    };

    outputs = { nixpkgs, ... } @ inputs: {
        devShells.x86_64-linux.default = let 
            pkgs = nixpkgs.legacyPackages.x86_64-linux;
        in pkgs.mkShell {
            packages = with pkgs; [
                pkgconf
                ninja
                cmake
                meson
                
                hidapi
                libvirt
                libevdev
            ];
        };
    };
}
{
	description = "Python C Extension Dev Environ";
	inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

	outputs = { self, nixpkgs }: let
		system = "x86_64-linux";
		pkgs = import nixpkgs { inherit system; };
	in {
		devShells.${system}.default = pkgs.mkShell {
			buildInputs = with pkgs; [
				uv
				stdenv.cc
				pkg-config
			];
			shellHook = ''
				export UV_PYTHON_PREFERENCE=only-managed
				if [ ! -d ".venv" ]; then
					uv venv
				fi
				source .venv/bin/activate
				echo "I have sourced the venv"
			'';
		};
	};
}

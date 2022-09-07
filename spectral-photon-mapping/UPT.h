#pragma once

enum AAType {
	BOX,
	GAUSS,
	Lanczos
};
namespace Spectral {
	namespace UPT {
		struct Settings {
		};
		class Tracer {


		};
	}
}

namespace Spectral {
	namespace Tristimulus {
		struct Settings {
			int iterations = 1;
			int samples = 1;
			float aaRadius = 0.5;
			int minBounces = 3;
			int maxBounces = 12;
			// todo: Not implemented
			int threads = 8;
			// todo: Not implemented
			int tileSize = 16;
		};
		class Tracer {


		};
	}
}
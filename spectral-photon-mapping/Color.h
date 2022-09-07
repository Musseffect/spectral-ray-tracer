#pragma once
#include <glm/common.hpp>
#include <functional>
#include <array>

#include "Function1D.h"
#include "ColorSampler.h"
#include "ColorSpaces.h"

using rgb = glm::vec3;

namespace Spectral {
	using Color = float;
}
namespace Tristimulus {
	using Color = rgb;
}

// todo: Munsell color chart

class ColorSampler;

namespace Tristimulus {
	namespace sRGB {
		extern const vec3 black;
		extern const vec3 white;
		extern const vec3 red;
		extern const vec3 green;
		extern const vec3 blue;
		extern const vec3 cyan;
		extern const vec3 magenta;
		extern const vec3 yellow;
		namespace ColorChecker {
			extern const vec3 colors[24];
			extern const vec3& darkSkin;
			extern const vec3& lightSkin;
			extern const vec3& bluSky;
			extern const vec3& foliage;
			extern const vec3& blueFlower;
			extern const vec3& bluishGreen;
			extern const vec3& orange;
			extern const vec3& purplishBlue;
			extern const vec3& moderateRed;
			extern const vec3& purple;
			extern const vec3& yellowGreen;
			extern const vec3& orangeYellow;
			extern const vec3& blue;
			extern const vec3& green;
			extern const vec3& red;
			extern const vec3& yellow;
			extern const vec3& magenta;
			extern const vec3& cyan;
			extern const vec3& white_9_5;
			extern const vec3& neutral_8;
			extern const vec3& neutral_6_5;
			extern const vec3& neutral_5;
			extern const vec3& neutral_3_5;
			extern const vec3& black_2;

			const vec3& colorByIndex(int index);
		};
	};
	namespace XYZ {
		namespace ColorChecker {
		}
	}
}

namespace Spectral {
	namespace ColorChecker {
		// E white point, to convert in sRGB use E -> D65 chromatic adaptation
		extern const spSpectrumSampler colors[24];
		const spSpectrumSampler& colorByIndex(int index);
	};
	// D65 illuminant
	namespace IT8 {
		extern const spSpectrumSampler colors[240];
		extern const spSpectrumSampler grayscale[24];

		extern const spSpectrumSampler& A1;
		extern const spSpectrumSampler& B1;
		extern const spSpectrumSampler& C1;
		extern const spSpectrumSampler& D1;
		extern const spSpectrumSampler& E1;
		extern const spSpectrumSampler& F1;
		extern const spSpectrumSampler& G1;
		extern const spSpectrumSampler& H1;
		extern const spSpectrumSampler& I1;
		extern const spSpectrumSampler& J1;
		extern const spSpectrumSampler& K1;
		extern const spSpectrumSampler& L1;

		extern const spSpectrumSampler& A2;
		extern const spSpectrumSampler& B2;
		extern const spSpectrumSampler& C2;
		extern const spSpectrumSampler& D2;
		extern const spSpectrumSampler& E2;
		extern const spSpectrumSampler& F2;
		extern const spSpectrumSampler& G2;
		extern const spSpectrumSampler& H2;
		extern const spSpectrumSampler& I2;
		extern const spSpectrumSampler& J2;
		extern const spSpectrumSampler& K2;
		extern const spSpectrumSampler& L2;

		extern const spSpectrumSampler& A3;
		extern const spSpectrumSampler& B3;
		extern const spSpectrumSampler& C3;
		extern const spSpectrumSampler& D3;
		extern const spSpectrumSampler& E3;
		extern const spSpectrumSampler& F3;
		extern const spSpectrumSampler& G3;
		extern const spSpectrumSampler& H3;
		extern const spSpectrumSampler& I3;
		extern const spSpectrumSampler& J3;
		extern const spSpectrumSampler& K3;
		extern const spSpectrumSampler& L3;

		extern const spSpectrumSampler& A4;
		extern const spSpectrumSampler& B4;
		extern const spSpectrumSampler& C4;
		extern const spSpectrumSampler& D4;
		extern const spSpectrumSampler& E4;
		extern const spSpectrumSampler& F4;
		extern const spSpectrumSampler& G4;
		extern const spSpectrumSampler& H4;
		extern const spSpectrumSampler& I4;
		extern const spSpectrumSampler& J4;
		extern const spSpectrumSampler& K4;
		extern const spSpectrumSampler& L4;

		extern const spSpectrumSampler& A5;
		extern const spSpectrumSampler& B5;
		extern const spSpectrumSampler& C5;
		extern const spSpectrumSampler& D5;
		extern const spSpectrumSampler& E5;
		extern const spSpectrumSampler& F5;
		extern const spSpectrumSampler& G5;
		extern const spSpectrumSampler& H5;
		extern const spSpectrumSampler& I5;
		extern const spSpectrumSampler& J5;
		extern const spSpectrumSampler& K5;
		extern const spSpectrumSampler& L5;

		extern const spSpectrumSampler& A6;
		extern const spSpectrumSampler& B6;
		extern const spSpectrumSampler& C6;
		extern const spSpectrumSampler& D6;
		extern const spSpectrumSampler& E6;
		extern const spSpectrumSampler& F6;
		extern const spSpectrumSampler& G6;
		extern const spSpectrumSampler& H6;
		extern const spSpectrumSampler& I6;
		extern const spSpectrumSampler& J6;
		extern const spSpectrumSampler& K6;
		extern const spSpectrumSampler& L6;

		extern const spSpectrumSampler& A7;
		extern const spSpectrumSampler& B7;
		extern const spSpectrumSampler& C7;
		extern const spSpectrumSampler& D7;
		extern const spSpectrumSampler& E7;
		extern const spSpectrumSampler& F7;
		extern const spSpectrumSampler& G7;
		extern const spSpectrumSampler& H7;
		extern const spSpectrumSampler& I7;
		extern const spSpectrumSampler& J7;
		extern const spSpectrumSampler& K7;
		extern const spSpectrumSampler& L7;

		extern const spSpectrumSampler& A8;
		extern const spSpectrumSampler& B8;
		extern const spSpectrumSampler& C8;
		extern const spSpectrumSampler& D8;
		extern const spSpectrumSampler& E8;
		extern const spSpectrumSampler& F8;
		extern const spSpectrumSampler& G8;
		extern const spSpectrumSampler& H8;
		extern const spSpectrumSampler& I8;
		extern const spSpectrumSampler& J8;
		extern const spSpectrumSampler& K8;
		extern const spSpectrumSampler& L8;

		extern const spSpectrumSampler& A9;
		extern const spSpectrumSampler& B9;
		extern const spSpectrumSampler& C9;
		extern const spSpectrumSampler& D9;
		extern const spSpectrumSampler& E9;
		extern const spSpectrumSampler& F9;
		extern const spSpectrumSampler& G9;
		extern const spSpectrumSampler& H9;
		extern const spSpectrumSampler& I9;
		extern const spSpectrumSampler& J9;
		extern const spSpectrumSampler& K9;
		extern const spSpectrumSampler& L9;

		extern const spSpectrumSampler& A10;
		extern const spSpectrumSampler& B10;
		extern const spSpectrumSampler& C10;
		extern const spSpectrumSampler& D10;
		extern const spSpectrumSampler& E10;
		extern const spSpectrumSampler& F10;
		extern const spSpectrumSampler& G10;
		extern const spSpectrumSampler& H10;
		extern const spSpectrumSampler& I10;
		extern const spSpectrumSampler& J10;
		extern const spSpectrumSampler& K10;
		extern const spSpectrumSampler& L10;

		extern const spSpectrumSampler& A11;
		extern const spSpectrumSampler& B11;
		extern const spSpectrumSampler& C11;
		extern const spSpectrumSampler& D11;
		extern const spSpectrumSampler& E11;
		extern const spSpectrumSampler& F11;
		extern const spSpectrumSampler& G11;
		extern const spSpectrumSampler& H11;
		extern const spSpectrumSampler& I11;
		extern const spSpectrumSampler& J11;
		extern const spSpectrumSampler& K11;
		extern const spSpectrumSampler& L11;

		extern const spSpectrumSampler& A12;
		extern const spSpectrumSampler& B12;
		extern const spSpectrumSampler& C12;
		extern const spSpectrumSampler& D12;
		extern const spSpectrumSampler& E12;
		extern const spSpectrumSampler& F12;
		extern const spSpectrumSampler& G12;
		extern const spSpectrumSampler& H12;
		extern const spSpectrumSampler& I12;
		extern const spSpectrumSampler& J12;
		extern const spSpectrumSampler& K12;
		extern const spSpectrumSampler& L12;

		extern const spSpectrumSampler& A13;
		extern const spSpectrumSampler& B13;
		extern const spSpectrumSampler& C13;
		extern const spSpectrumSampler& D13;
		extern const spSpectrumSampler& E13;
		extern const spSpectrumSampler& F13;
		extern const spSpectrumSampler& G13;
		extern const spSpectrumSampler& H13;
		extern const spSpectrumSampler& I13;
		extern const spSpectrumSampler& J13;
		extern const spSpectrumSampler& K13;
		extern const spSpectrumSampler& L13;

		extern const spSpectrumSampler& A14;
		extern const spSpectrumSampler& B14;
		extern const spSpectrumSampler& C14;
		extern const spSpectrumSampler& D14;
		extern const spSpectrumSampler& E14;
		extern const spSpectrumSampler& F14;
		extern const spSpectrumSampler& G14;
		extern const spSpectrumSampler& H14;
		extern const spSpectrumSampler& I14;
		extern const spSpectrumSampler& J14;
		extern const spSpectrumSampler& K14;
		extern const spSpectrumSampler& L14;

		extern const spSpectrumSampler& A15;
		extern const spSpectrumSampler& B15;
		extern const spSpectrumSampler& C15;
		extern const spSpectrumSampler& D15;
		extern const spSpectrumSampler& E15;
		extern const spSpectrumSampler& F15;
		extern const spSpectrumSampler& G15;
		extern const spSpectrumSampler& H15;
		extern const spSpectrumSampler& I15;
		extern const spSpectrumSampler& J15;
		extern const spSpectrumSampler& K15;
		extern const spSpectrumSampler& L15;

		extern const spSpectrumSampler& A16;
		extern const spSpectrumSampler& B16;
		extern const spSpectrumSampler& C16;
		extern const spSpectrumSampler& D16;
		extern const spSpectrumSampler& E16;
		extern const spSpectrumSampler& F16;
		extern const spSpectrumSampler& G16;
		extern const spSpectrumSampler& H16;
		extern const spSpectrumSampler& I16;
		extern const spSpectrumSampler& J16;
		extern const spSpectrumSampler& K16;
		extern const spSpectrumSampler& L16;

		extern const spSpectrumSampler& A17;
		extern const spSpectrumSampler& B17;
		extern const spSpectrumSampler& C17;
		extern const spSpectrumSampler& D17;
		extern const spSpectrumSampler& E17;
		extern const spSpectrumSampler& F17;
		extern const spSpectrumSampler& G17;
		extern const spSpectrumSampler& H17;
		extern const spSpectrumSampler& I17;
		extern const spSpectrumSampler& J17;
		extern const spSpectrumSampler& K17;
		extern const spSpectrumSampler& L17;

		extern const spSpectrumSampler& A18;
		extern const spSpectrumSampler& B18;
		extern const spSpectrumSampler& C18;
		extern const spSpectrumSampler& D18;
		extern const spSpectrumSampler& E18;
		extern const spSpectrumSampler& F18;
		extern const spSpectrumSampler& G18;
		extern const spSpectrumSampler& H18;
		extern const spSpectrumSampler& I18;
		extern const spSpectrumSampler& J18;
		extern const spSpectrumSampler& K18;
		extern const spSpectrumSampler& L18;

		extern const spSpectrumSampler& A19;
		extern const spSpectrumSampler& B19;
		extern const spSpectrumSampler& C19;
		extern const spSpectrumSampler& D19;
		extern const spSpectrumSampler& E19;
		extern const spSpectrumSampler& F19;
		extern const spSpectrumSampler& G19;
		extern const spSpectrumSampler& H19;
		extern const spSpectrumSampler& I19;
		extern const spSpectrumSampler& J19;
		extern const spSpectrumSampler& K19;
		extern const spSpectrumSampler& L19;

		extern const spSpectrumSampler& I20;
		extern const spSpectrumSampler& J20;
		extern const spSpectrumSampler& K20;
		extern const spSpectrumSampler& L20;

		extern const spSpectrumSampler& I21;
		extern const spSpectrumSampler& J21;
		extern const spSpectrumSampler& K21;
		extern const spSpectrumSampler& L21;

		extern const spSpectrumSampler& I22;
		extern const spSpectrumSampler& J22;
		extern const spSpectrumSampler& K22;
		extern const spSpectrumSampler& L22;
	}
}
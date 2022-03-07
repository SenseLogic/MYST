/*
    This file is part of the Myst distribution.

    https://github.com/senselogic/MYST

    Copyright (C) 2021 Eric Pelzer (ecstatic.coder@gmail.com)

    Myst is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Myst is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Myst.  If not, see <http://www.gnu.org/licenses/>.
*/

// -- IMPORTS

#include <math.h>
#include <string.h>
#include <iostream>
#include <fstream>

typedef struct _SYSTEMTIME
{
  uint16_t wYear;
  uint16_t wMonth;
  uint16_t wDayOfWeek;
  uint16_t wDay;
  uint16_t wHour;
  uint16_t wMinute;
  uint16_t wSecond;
  uint16_t wMilliseconds;
} SYSTEMTIME;

#include "E57Foundation.h"
#include "E57Simple.h"

#include "base.hpp"
#include "cell.hpp"
#include "cloud.hpp"
#include "compression.hpp"
#include "link_.hpp"
#include "scan.hpp"
#include "vector_3.hpp"
#include "vector_4.hpp"

using namespace std;
using namespace e57;
using base::Abort;
using base::GetText;
using base::PrintProgress;
using base::LINK_;
using base::VECTOR_3;
using base::VECTOR_4;
using pcf::CELL;
using pcf::CLOUD;
using pcf::COMPONENT;
using pcf::COMPRESSION;
using pcf::SCAN;

// -- CONSTANTS

const int64_t
    BufferPointCount = 65536;

// -- VARIABLES

static double
    PointAComponentArray[ BufferPointCount ],
    PointDComponentArray[ BufferPointCount ],
    PointEComponentArray[ BufferPointCount ],
    PointIComponentArray[ BufferPointCount ],
    PointXComponentArray[ BufferPointCount ],
    PointYComponentArray[ BufferPointCount ],
    PointZComponentArray[ BufferPointCount ];
static int8_t
    PointInvalidDaeComponentArray[ BufferPointCount ],
    PointInvalidIComponentArray[ BufferPointCount ],
    PointInvalidRgbComponentArray[ BufferPointCount ],
    PointInvalidXyzComponentArray[ BufferPointCount ];
static uint16_t
    PointBComponentArray[ BufferPointCount ],
    PointGComponentArray[ BufferPointCount ],
    PointRComponentArray[ BufferPointCount ];
static bool
    VerboseOptionIsEnabled,
    ProgressOptionIsEnabled;

// -- TYPES

struct TRANSFORM
{
    // -- ATTRIBUTES

    bool
        SwapsXY,
        SwapsXZ,
        SwapsYZ;
    VECTOR_3
        PositionOffsetVector,
        PositionRotationVector,
        PositionScalingVector,
        PositionTranslationVector;
    VECTOR_4
        ColorOffsetVector,
        ColorScalingVector,
        ColorTranslationVector;
    bool
        AppliesIntensity;

    // -- CONSTRUCTORS

    TRANSFORM(
        ) :
        SwapsXY( false ),
        SwapsXZ( false ),
        SwapsYZ( false ),
        PositionOffsetVector( 0.0, 0.0, 0.0 ),
        PositionRotationVector( 0.0, 0.0, 0.0 ),
        PositionScalingVector( 1.0, 1.0, 1.0 ),
        PositionTranslationVector( 0.0, 0.0, 0.0 ),
        ColorOffsetVector( 0.0, 0.0, 0.0, 0.0 ),
        ColorScalingVector( 1.0, 1.0, 1.0, 1.0 ),
        ColorTranslationVector( 0.0, 0.0, 0.0, 0.0 ),
        AppliesIntensity( false )
    {
    }
};

// ~~

struct POINT
{
    // -- ATTRIBUTES

    VECTOR_3
        SphericalPositionVector,
        PositionVector;
    VECTOR_4
        ColorVector;

    // -- CONSTRUCTORS

    POINT(
        ) :
        SphericalPositionVector( 0.0, 0.0, 0.0 ),
        PositionVector( 0.0, 0.0, 0.0 ),
        ColorVector( 0.0, 0.0, 0.0, 0.0 )
    {
    }

    // -- OPERATORS

    bool operator==(
        const POINT & point
        ) const
    {
        return
            SphericalPositionVector == point.SphericalPositionVector
            && PositionVector == point.PositionVector
            && ColorVector == point.ColorVector;
    }

    // ~~

    bool operator!=(
        const POINT & point
        ) const
    {
        return
            SphericalPositionVector != point.SphericalPositionVector
            || PositionVector != point.PositionVector
            || ColorVector != point.ColorVector;
    }

    // -- INQUIRIES

    void Dump(
        string indentation = ""
        ) const
    {
        cout
            << indentation
            << GetText( SphericalPositionVector )
            << " "
            << GetText( PositionVector )
            << " "
            << GetText( ColorVector )
            << "\n";
    }

    // ~~

    double GetComponentValue(
        char component_character
        )
    {
        if ( component_character == 'D' )
        {
            return SphericalPositionVector.X;
        }
        else if ( component_character == 'A' )
        {
            return SphericalPositionVector.Y;
        }
        else if ( component_character == 'E' )
        {
            return SphericalPositionVector.Z;
        }
        else if ( component_character == 'x' )
        {
            return -PositionVector.X;
        }
        else if ( component_character == 'X' )
        {
            return PositionVector.X;
        }
        else if ( component_character == 'y' )
        {
            return -PositionVector.Y;
        }
        else if ( component_character == 'Y' )
        {
            return PositionVector.Y;
        }
        else if ( component_character == 'z' )
        {
            return -PositionVector.Z;
        }
        else if ( component_character == 'Z' )
        {
            return PositionVector.Z;
        }
        else if ( component_character == 'n' )
        {
            return floor( -2048.0 + ColorVector.W * 4095.0 );
        }
        else if ( component_character == 'i' )
        {
            return ColorVector.W;
        }
        else if ( component_character == 'I' )
        {
            return floor( ColorVector.W * 255.0 );
        }
        else if ( component_character == 'r' )
        {
            return ColorVector.X;
        }
        else if ( component_character == 'R' )
        {
            return floor( ColorVector.X * 255.0 );
        }
        else if ( component_character == 'g' )
        {
            return ColorVector.Y;
        }
        else if ( component_character == 'G' )
        {
            return floor( ColorVector.Y * 255.0 );
        }
        else if ( component_character == 'b' )
        {
            return ColorVector.Z;
        }
        else if ( component_character == 'B' )
        {
            return floor( ColorVector.Z * 255.0 );
        }
        else
        {
            Abort( "Invalid component format" );

            return 0.0;
        }
    }

    // ~~

    POINT GetTransformedPoint(
        const TRANSFORM & transform
        ) const
    {
        POINT
            transformed_point;


        transformed_point = *this;

        if ( transform.SwapsXY )
        {
            transformed_point.PositionVector.SetVector(
                transformed_point.PositionVector.Y,
                transformed_point.PositionVector.X,
                transformed_point.PositionVector.Z
                );
        }

        if ( transform.SwapsXZ )
        {
            transformed_point.PositionVector.SetVector(
                transformed_point.PositionVector.Z,
                transformed_point.PositionVector.Y,
                transformed_point.PositionVector.X
                );
        }

        if ( transform.SwapsYZ )
        {
            transformed_point.PositionVector.SetVector(
                transformed_point.PositionVector.X,
                transformed_point.PositionVector.Z,
                transformed_point.PositionVector.Y
                );
        }

        transformed_point.PositionVector.Translate(
            transform.PositionOffsetVector.X,
            transform.PositionOffsetVector.Y,
            transform.PositionOffsetVector.Z
            );

        transformed_point.PositionVector.Scale(
            transform.PositionScalingVector.X,
            transform.PositionScalingVector.Y,
            transform.PositionScalingVector.Z
            );

        if ( transform.PositionRotationVector.Z != 0.0 )
        {
            transformed_point.PositionVector.RotateAroundZ(
                cos( transform.PositionRotationVector.Z ),
                sin( transform.PositionRotationVector.Z )
                );
        }

        if ( transform.PositionRotationVector.X != 0.0 )
        {
            transformed_point.PositionVector.RotateAroundX(
                cos( transform.PositionRotationVector.X ),
                sin( transform.PositionRotationVector.X )
                );
        }

        if ( transform.PositionRotationVector.Y != 0.0 )
        {
            transformed_point.PositionVector.RotateAroundY(
                cos( transform.PositionRotationVector.Y ),
                sin( transform.PositionRotationVector.Y )
                );
        }

        transformed_point.PositionVector.Translate(
            transform.PositionTranslationVector.X,
            transform.PositionTranslationVector.Y,
            transform.PositionTranslationVector.Z
            );

        transformed_point.ColorVector.Translate(
            transform.ColorOffsetVector.X,
            transform.ColorOffsetVector.Y,
            transform.ColorOffsetVector.Z,
            transform.ColorOffsetVector.W
            );

        transformed_point.ColorVector.Scale(
            transform.ColorScalingVector.X,
            transform.ColorScalingVector.Y,
            transform.ColorScalingVector.Z,
            transform.ColorScalingVector.W
            );

        transformed_point.ColorVector.Translate(
            transform.ColorTranslationVector.X,
            transform.ColorTranslationVector.Y,
            transform.ColorTranslationVector.Z,
            transform.ColorTranslationVector.W
            );

        if ( transform.AppliesIntensity )
        {
            transformed_point.ColorVector.Scale(
                transformed_point.ColorVector.W,
                transformed_point.ColorVector.W,
                transformed_point.ColorVector.W,
                1.0
                );
        }

        transformed_point.ColorVector.Clamp( 0.0, 1.0 );

        return transformed_point;
    }
};

// ~~

struct E57_SCAN
{
    // -- ATTRIBUTES

    string
        Name;
    VECTOR_3
        PositionVector;
    VECTOR_4
        RotationVector;
    VECTOR_3
        XAxisVector,
        YAxisVector,
        ZAxisVector,
        ScalingVector;
    Data3D
        Data;
    int64_t
        RowCount,
        ColumnCount,
        PointCount,
        GroupCount,
        BufferPointCount;
    bool
        IsColumnIndex,
        HasDComponent,
        HasAComponent,
        HasEComponent,
        HasInvalidDaeComponent,
        HasXComponent,
        HasYComponent,
        HasZComponent,
        HasInvalidXyzComponent,
        HasIComponent,
        HasInvalidIComponent,
        HasRComponent,
        HasGComponent,
        HasBComponent,
        HasInvalidRgbComponent;
    double
        MinimumD,
        MaximumD,
        MinimumA,
        MaximumA,
        MinimumE,
        MaximumE,
        MinimumX,
        MaximumX,
        MinimumY,
        MaximumY,
        MinimumZ,
        MaximumZ,
        MinimumI,
        MaximumI,
        MinimumR,
        MaximumR,
        MinimumG,
        MaximumG,
        MinimumB,
        MaximumB;

    // -- CONSTRUCTORS

    E57_SCAN(
        ) :
        Name(),
        PositionVector(),
        RotationVector(),
        XAxisVector(),
        YAxisVector(),
        ZAxisVector(),
        ScalingVector( 1.0, 1.0, 1.0 ),
        Data(),
        RowCount( 0 ),
        ColumnCount( 0 ),
        PointCount( 0 ),
        GroupCount( 0 ),
        BufferPointCount( 0 ),
        IsColumnIndex( false ),
        HasDComponent( false ),
        HasAComponent( false ),
        HasEComponent( false ),
        HasInvalidDaeComponent( false ),
        HasXComponent( false ),
        HasYComponent( false ),
        HasZComponent( false ),
        HasInvalidXyzComponent( false ),
        HasIComponent( false ),
        HasInvalidIComponent( false ),
        HasRComponent( false ),
        HasGComponent( false ),
        HasBComponent( false ),
        HasInvalidRgbComponent( false ),
        MinimumD( 0.0 ),
        MaximumD( 0.0 ),
        MinimumA( 0.0 ),
        MaximumA( 0.0 ),
        MinimumE( 0.0 ),
        MaximumE( 0.0 ),
        MinimumX( 0.0 ),
        MaximumX( 0.0 ),
        MinimumY( 0.0 ),
        MaximumY( 0.0 ),
        MinimumZ( 0.0 ),
        MaximumZ( 0.0 ),
        MinimumI( 0.0 ),
        MaximumI( 0.0 ),
        MinimumR( 0.0 ),
        MaximumR( 0.0 ),
        MinimumG( 0.0 ),
        MaximumG( 0.0 ),
        MinimumB( 0.0 ),
        MaximumB( 0.0 )
    {
    }

    // -- INQUIRIES

    bool IsValidPoint(
        int64_t point_index
        )
    {
        return
            ( !HasInvalidDaeComponent || !PointInvalidDaeComponentArray[ point_index ])
            && ( !HasInvalidXyzComponent || !PointInvalidXyzComponentArray[ point_index ])
            && ( !HasInvalidIComponent || !PointInvalidIComponentArray[ point_index ])
            && ( !HasInvalidRgbComponent || !PointInvalidRgbComponentArray[ point_index ]);

    }

    // ~~

    void Dump(
        string indentation = ""
        ) const
    {
        cout << indentation << "Name : " << Name << "\n";
        cout << indentation << "PositionVector : " << GetText( PositionVector ) << "\n";
        cout << indentation << "RotationVector : " << GetText( RotationVector ) << "\n";
        cout << indentation << "XAxisVector : " << GetText( XAxisVector ) << "\n";
        cout << indentation << "YAxisVector : " << GetText( YAxisVector ) << "\n";
        cout << indentation << "ZAxisVector : " << GetText( ZAxisVector ) << "\n";
        cout << indentation << "RowCount : " << RowCount << "\n";
        cout << indentation << "ColumnCount : " << ColumnCount << "\n";
        cout << indentation << "PointCount : " << PointCount << "\n";

        if ( HasDComponent )
        {
            cout << indentation << "D : " << GetText( MinimumD ) << " / " << GetText( MaximumD ) << "\n";
        }

        if ( HasAComponent )
        {
            cout << indentation << "A : " << GetText( MinimumA ) << " / " << GetText( MaximumA ) << "\n";
        }

        if ( HasEComponent )
        {
            cout << indentation << "E : " << GetText( MinimumE ) << " / " << GetText( MaximumE ) << "\n";
        }

        if ( HasXComponent )
        {
            cout << indentation << "X : " << GetText( MinimumX ) << " / " << GetText( MaximumX ) << "\n";
        }

        if ( HasYComponent )
        {
            cout << indentation << "Y : " << GetText( MinimumY ) << " / " << GetText( MaximumY ) << "\n";
        }

        if ( HasZComponent )
        {
            cout << indentation << "Z : " << GetText( MinimumZ ) << " / " << GetText( MaximumZ ) << "\n";
        }

        if ( HasIComponent )
        {
            cout << indentation << "I : " << GetText( MinimumI ) << " / " << GetText( MaximumI ) << "\n";
        }

        if ( HasRComponent )
        {
            cout << indentation << "R : " << GetText( MinimumR ) << " / " << GetText( MaximumR ) << "\n";
        }

        if ( HasGComponent )
        {
            cout << indentation << "G : " << GetText( MinimumG ) << " / " << GetText( MaximumG ) << "\n";
        }

        if ( HasBComponent )
        {
            cout << indentation << "B : " << GetText( MinimumB ) << " / " << GetText( MaximumB ) << "\n";
        }
    }

    // -- OPERATIONS

    void ReadData(
        Reader & reader,
        int32_t scan_index
        )
    {
        reader.ReadData3D(
            scan_index,
            Data
            );

        reader.GetData3DSizes(
            scan_index,
            RowCount,
            ColumnCount,
            PointCount,
            GroupCount,
            BufferPointCount,
            IsColumnIndex
            );

        cout
            << "Scan "
            << scan_index
            << " : "
            << Data.name
            << " ("
            << PointCount
            << " points, "
            << ColumnCount
            << " rows, "
            << ColumnCount
            << " columns)\n";

        if ( RowCount == 0
             && ColumnCount == 0
             && PointCount > 0 )
        {
            RowCount = 1;
            ColumnCount = PointCount;
        }

        Name = Data.name;
        PositionVector.X = Data.pose.translation.x;
        PositionVector.Y = Data.pose.translation.y;
        PositionVector.Z = Data.pose.translation.z;
        RotationVector.X = Data.pose.rotation.x;
        RotationVector.Y = Data.pose.rotation.y;
        RotationVector.Z = Data.pose.rotation.z;
        RotationVector.W = Data.pose.rotation.w;
        RotationVector.GetAxisVectors( XAxisVector, YAxisVector, ZAxisVector );

        HasDComponent = Data.pointFields.sphericalRangeField;
        HasAComponent = Data.pointFields.sphericalAzimuthField;
        HasEComponent = Data.pointFields.sphericalElevationField;
        HasInvalidDaeComponent = Data.pointFields.sphericalInvalidStateField;
        HasXComponent = Data.pointFields.cartesianXField;
        HasYComponent = Data.pointFields.cartesianYField;
        HasZComponent = Data.pointFields.cartesianZField;
        HasInvalidXyzComponent = Data.pointFields.cartesianInvalidStateField;
        HasIComponent = Data.pointFields.intensityField;
        HasInvalidIComponent = Data.pointFields.isIntensityInvalidField;
        HasRComponent = Data.pointFields.colorRedField;
        HasGComponent = Data.pointFields.colorGreenField;
        HasBComponent = Data.pointFields.colorBlueField;
        HasInvalidRgbComponent = Data.pointFields.isColorInvalidField;

        MinimumD = Data.sphericalBounds.rangeMinimum;
        MaximumD = Data.sphericalBounds.rangeMaximum;
        MinimumA = Data.sphericalBounds.azimuthStart;
        MaximumA = Data.sphericalBounds.azimuthEnd;
        MinimumE = Data.sphericalBounds.elevationMinimum;
        MaximumE = Data.sphericalBounds.elevationMaximum;
        MinimumX = Data.cartesianBounds.xMinimum;
        MaximumX = Data.cartesianBounds.xMaximum;
        MinimumY = Data.cartesianBounds.yMinimum;
        MaximumY = Data.cartesianBounds.yMaximum;
        MinimumZ = Data.cartesianBounds.zMinimum;
        MaximumZ = Data.cartesianBounds.zMaximum;
        MinimumI = Data.intensityLimits.intensityMinimum;
        MaximumI = Data.intensityLimits.intensityMaximum;
        MinimumR = Data.colorLimits.colorRedMinimum;
        MaximumR = Data.colorLimits.colorRedMaximum;
        MinimumG = Data.colorLimits.colorGreenMinimum;
        MaximumG = Data.colorLimits.colorGreenMaximum;
        MinimumB = Data.colorLimits.colorBlueMinimum;
        MaximumB = Data.colorLimits.colorBlueMaximum;
    }

    // ~~

    void SetPoint(
        POINT & point,
        int64_t point_index,
        bool point_is_transformed
        )
    {
        if ( HasDComponent )
        {
            point.SphericalPositionVector.X = PointDComponentArray[ point_index ];
        }

        if ( HasAComponent )
        {
            point.SphericalPositionVector.Y = PointAComponentArray[ point_index ];
        }

        if ( HasEComponent )
        {
            point.SphericalPositionVector.Z = PointEComponentArray[ point_index ];
        }

        if ( HasXComponent )
        {
            point.PositionVector.X = PointXComponentArray[ point_index ];
        }

        if ( HasYComponent )
        {
            point.PositionVector.Y = PointYComponentArray[ point_index ];
        }

        if ( HasZComponent )
        {
            point.PositionVector.Z = PointZComponentArray[ point_index ];
        }

        if ( HasIComponent )
        {
            point.ColorVector.W = ( PointIComponentArray[ point_index ] - MinimumI ) / ( MaximumI - MinimumI );
        }

        if ( HasRComponent )
        {
            point.ColorVector.X = ( PointRComponentArray[ point_index ] - MinimumR ) / ( MaximumR - MinimumR );
        }

        if ( HasGComponent )
        {
            point.ColorVector.Y = ( PointGComponentArray[ point_index ] - MinimumG ) / ( MaximumG - MinimumG );
        }

        if ( HasBComponent )
        {
            point.ColorVector.Z = ( PointBComponentArray[ point_index ] - MinimumB ) / ( MaximumB - MinimumB );
        }

        if ( HasDComponent
             && HasAComponent
             && HasEComponent
             && !HasXComponent
             && !HasYComponent
             && !HasZComponent )
        {
            point.PositionVector.SetFromSphericalPosition(
                point.SphericalPositionVector.X,
                point.SphericalPositionVector.Y,
                point.SphericalPositionVector.Z,
                true
                );
        }

        if ( point_is_transformed )
        {
            point.PositionVector.ApplyTranslationRotationScalingTransform(
                PositionVector,
                XAxisVector,
                YAxisVector,
                ZAxisVector,
                ScalingVector
                );
        }
    }
};

// ~~

struct E57_CLOUD
{
    // -- ATTRIBUTES

    string
        AxisFormat;
    bool
        IntensityIsApplied,
        PointIsTransformed;
    TRANSFORM
        Transform;
    vector<string>
        IgnoredScanNameVector,
        SelectedScanNameVector;
    vector<E57_SCAN>
        ScanVector;
    int64_t
        MaximumScanCount,
        MaximumScanPointCount,
        PointDecimationCount;

    // -- CONSTRUCTORS

    E57_CLOUD(
        ) :
        AxisFormat( "XYZ" ),
        IntensityIsApplied( false ),
        PointIsTransformed( false ),
        Transform(),
        IgnoredScanNameVector(),
        SelectedScanNameVector(),
        ScanVector(),
        PointDecimationCount( 1 )
    {
    }

    // -- INQUIRIES

    string GetComponentName(
        char component_character
        )
    {
        if ( component_character == 'D' )
        {
            return "X";
        }
        else if ( component_character == 'A' )
        {
            return "A";
        }
        else if ( component_character == 'E' )
        {
            return "E";
        }
        else if ( component_character == 'x'
             || component_character == 'X' )
        {
            return "X";
        }
        else if ( component_character == 'y'
                 || component_character == 'Y' )
        {
            return "Y";
        }
        else if ( component_character == 'z'
                 || component_character == 'Z' )
        {
            return "Z";
        }
        else if ( component_character == 'n'
                 || component_character == 'i'
                 || component_character == 'I' )
        {
            return "I";
        }
        else if ( component_character == 'r'
                 || component_character == 'R' )
        {
            return "R";
        }
        else if ( component_character == 'g'
                 || component_character == 'G' )
        {
            return "G";
        }
        else if ( component_character == 'b'
                 || component_character == 'B' )
        {
            return "B";
        }
        else
        {
            Abort( "Invalid component format" );

            return "";
        }
    }

    // ~~

    void TransformPoint(
        POINT & point
        )
    {
        if ( VerboseOptionIsEnabled )
        {
            point.Dump();
        }

        if ( PointIsTransformed )
        {
            point = point.GetTransformedPoint( Transform );

            if ( VerboseOptionIsEnabled )
            {
                cout << "=> ";
                point.Dump();
            }
        }
    }

    // ~~

    int64_t GetScanPointCount(
        const E57_SCAN & e57_scan
        )
    {
        int64_t
            scan_point_count;

        scan_point_count = e57_scan.PointCount;

        return scan_point_count / PointDecimationCount;
    }

    // ~~

    bool IsSelectedScan(
        const E57_SCAN & e57_scan
        )
    {
        uint64_t
            ignored_scan_name_index,
            selected_scan_name_index;

        for ( ignored_scan_name_index = 0;
              ignored_scan_name_index < IgnoredScanNameVector.size();
              ++ignored_scan_name_index )
        {
            if ( e57_scan.Name == IgnoredScanNameVector[ ignored_scan_name_index ] )
            {
                return false;
            }
        }

        if ( SelectedScanNameVector.size() > 0 )
        {
            for ( selected_scan_name_index = 0;
                  selected_scan_name_index < SelectedScanNameVector.size();
                  ++selected_scan_name_index )
            {
                if ( e57_scan.Name == SelectedScanNameVector[ selected_scan_name_index ] )
                {
                    return true;
                }
            }

            return false;
        }
        else
        {
            return true;
        }
    }

    // -- OPERATIONS


    void ListScans(
        const string & input_file_path
        )
    {
        int32_t
            scan_count,
            scan_index;
        E57_SCAN
            scan;

        cout << "Reading file : " << input_file_path << "\n";

        Reader
            reader( input_file_path );

        scan_count = reader.GetData3DCount();

        cout << "ScanCount : " << scan_count << "\n";

        for ( scan_index = 0;
              scan_index < scan_count;
              ++scan_index )
        {
            scan.ReadData( reader, scan_index );
        }
    }

    // ~~

    void WriteXyzFile(
        const string & input_file_path,
        const string & output_file_path,
        const string & output_component_format,
        const string & output_file_format
        )
    {
        bool
            point_is_transformed;
        int32_t
            scan_count,
            scan_index;
        int64_t
            point_count,
            point_index,
            scan_point_count,
            scan_point_index;
        uint64_t
            component_count,
            component_index,
            progress;
        ofstream
            output_file_stream;
        POINT
            point;
        E57_SCAN
            * scan;

        component_count = output_component_format.size();

        cout << "Reading file : " << input_file_path << "\n";

        Reader
            reader( input_file_path );

        scan_count = reader.GetData3DCount();
        ScanVector.resize( scan_count );

        cout << "ScanCount : " << scan_count << "\n";

        point_count = 0;

        for ( scan_index = 0;
              scan_index < scan_count;
              ++scan_index )
        {
            scan = &ScanVector[ scan_index ];
            scan->ReadData( reader, scan_index );

            if ( IsSelectedScan( *scan ) )
            {
                point_count += GetScanPointCount( *scan );
            }
        }

        cout << "Writing file : " << output_file_path << "\n";

        output_file_stream.open( output_file_path );

        if ( output_file_format == "pts" )
        {
            output_file_stream << point_count << "\n";
        }

        for ( scan_index = 0;
              scan_index < scan_count;
              ++scan_index )
        {
            scan = &ScanVector[ scan_index ];

            if ( IsSelectedScan( *scan ) )
            {
                cout << "Scan[" << scan_index << "] : \n";
                scan->Dump( "    " );

                if ( output_file_format == "ptx" )
                {
                    scan_point_count = GetScanPointCount( *scan );

                    if ( scan_point_count < scan->ColumnCount * scan->RowCount )
                    {
                        scan->RowCount = 1;
                        scan->ColumnCount = scan_point_count;
                    }

                    output_file_stream
                        << scan->ColumnCount
                        << "\n"
                        << scan->RowCount
                        << "\n"
                        << scan->PositionVector.X
                        << " "
                        << scan->PositionVector.Y
                        << " "
                        << scan->PositionVector.Z
                        << "\n"
                        << scan->XAxisVector.X
                        << " "
                        << scan->XAxisVector.Y
                        << " "
                        << scan->XAxisVector.Z
                        << "\n"
                        << scan->YAxisVector.X
                        << " "
                        << scan->YAxisVector.Y
                        << " "
                        << scan->YAxisVector.Z
                        << "\n"
                        << scan->ZAxisVector.X
                        << " "
                        << scan->ZAxisVector.Y
                        << " "
                        << scan->ZAxisVector.Z
                        << "\n"
                        << scan->XAxisVector.X
                        << " "
                        << scan->XAxisVector.Y
                        << " "
                        << scan->XAxisVector.Z
                        << " 0\n"
                        << scan->YAxisVector.X
                        << " "
                        << scan->YAxisVector.Y
                        << " "
                        << scan->YAxisVector.Z
                        << " 0\n"
                        << scan->ZAxisVector.X
                        << " "
                        << scan->ZAxisVector.Y
                        << " "
                        << scan->ZAxisVector.Z
                        << " 0\n"
                        << scan->PositionVector.X
                        << " "
                        << scan->PositionVector.Y
                        << " "
                        << scan->PositionVector.Z
                        << " 1\n";

                    point_is_transformed = false;
                }
                else
                {
                    point_is_transformed = true;
                }

                CompressedVectorReader
                    compressed_vector_reader
                        = reader.SetUpData3DPointsData(
                              scan_index,
                              BufferPointCount,
                              PointXComponentArray,
                              PointYComponentArray,
                              PointZComponentArray,
                              PointInvalidXyzComponentArray,
                              PointIComponentArray,
                              PointInvalidIComponentArray,
                              PointRComponentArray,
                              PointGComponentArray,
                              PointBComponentArray,
                              PointInvalidRgbComponentArray,
                              PointDComponentArray,
                              PointAComponentArray,
                              PointEComponentArray,
                              PointInvalidDaeComponentArray
                              );

                progress = -1;
                scan_point_index = 0;

                while ( ( point_count = compressed_vector_reader.read() ) > 0 )
                {
                    for ( point_index = 0;
                          point_index < point_count;
                          ++point_index )
                    {
                        if ( scan->IsValidPoint( point_index )
                             && scan_point_index % PointDecimationCount == 0 )
                        {
                            scan->SetPoint( point, point_index, point_is_transformed );
                            TransformPoint( point );

                            for ( component_index = 0;
                                  component_index < component_count;
                                  ++component_index )
                            {
                                if ( component_index > 0 )
                                {
                                    output_file_stream << " ";
                                }

                                output_file_stream << point.GetComponentValue( output_component_format[ component_index ] );
                            }

                            output_file_stream << "\n";
                        }

                        if ( ProgressOptionIsEnabled )
                        {
                            PrintProgress( progress, scan_point_index, scan->PointCount );
                        }

                        ++scan_point_index;
                    }
                }

                if ( scan_point_index != scan->PointCount )
                {
                    cerr << "*** WARNING : " << scan_point_index << " points (" << scan->PointCount << ")\n";
                }
            }
        }

        output_file_stream.close();
    }

    // ~~

    void WritePcfFile(
        const string & input_file_path,
        const string & output_file_path,
        const string & output_component_format,
        const COMPRESSION compression,
        const uint16_t position_bit_count,
        const double position_precision
        )
    {
        char
            component_character,
            x_component_character,
            y_component_character,
            z_component_character;
        int32_t
            scan_count,
            scan_index;
        int64_t
            point_count,
            point_index,
            scan_point_index;
        uint64_t
            component_count,
            component_index,
            progress;
        LINK_<pcf::SCAN>
            pcf_scan;
        LINK_<pcf::CLOUD>
            pcf_cloud;
        pcf::CELL
            * pcf_cell;
        POINT
            point;
        E57_SCAN
            scan;

        cout << "Reading file : " << input_file_path << "\n";

        pcf_cloud = new pcf::CLOUD();
        pcf_cloud->SetAxisFormat( AxisFormat );

        Reader
            reader( input_file_path );

        component_count = output_component_format.size();

        scan_count = reader.GetData3DCount();
        ScanVector.resize( scan_count );

        cout << "ScanCount : " << scan_count << "\n";

        for ( scan_index = 0;
              scan_index < scan_count;
              ++scan_index )
        {
            scan.ReadData( reader, scan_index );

            if ( IsSelectedScan( scan ) )
            {
                cout << "Scan[" << scan_index << "] : \n";
                scan.Dump( "    " );

                pcf_scan = new SCAN();
                pcf_scan->Name = scan.Name;
                pcf_scan->PointCount = GetScanPointCount( scan );
                pcf_scan->ColumnCount = scan.ColumnCount;
                pcf_scan->RowCount = scan.RowCount;
                pcf_scan->PositionVector = scan.PositionVector;
                pcf_scan->RotationVector = scan.RotationVector;
                pcf_scan->SetAxisVectors();

                x_component_character = 'X';
                y_component_character = 'Y';
                z_component_character = 'Z';

                for ( component_index = 0;
                      component_index < component_count;
                      ++component_index )
                {
                    component_character = output_component_format[ component_index ];

                    if ( compression == COMPRESSION::None )
                    {
                        pcf_scan->ComponentVector.push_back(
                            new COMPONENT( GetComponentName( component_character ), COMPRESSION::None, 32, 0.0, 0.0, 0.0 )
                            );
                    }
                    else
                    {
                        assert( compression == COMPRESSION::Discretization );

                        if ( component_character == 'x'
                             || component_character == 'X' )
                        {
                            x_component_character = component_character;

                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "X", COMPRESSION::Discretization, position_bit_count, position_precision, 0.0, scan.MinimumX, scan.MaximumX )
                                );
                        }
                        else if ( component_character == 'y'
                                  || component_character == 'Y' )
                        {
                            y_component_character = component_character;

                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "Y", COMPRESSION::Discretization, position_bit_count, position_precision, 0.0, scan.MinimumY, scan.MaximumY )
                                );
                        }
                        else if ( component_character == 'z'
                                  || component_character == 'Z' )
                        {
                            z_component_character = component_character;

                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "Z", COMPRESSION::Discretization, position_bit_count, position_precision, 0.0, scan.MinimumZ, scan.MaximumZ )
                                );
                        }
                        else if ( component_character == 'n' )
                        {
                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "I", COMPRESSION::Discretization, 12, 1.0, -2048.0, -2048.0, 2047.0 )
                                );
                        }
                        else if ( component_character == 'i' )
                        {
                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "I", COMPRESSION::Discretization, 8, 1.0 / 255.0, 0.0, 0.0, 1.0 )
                                );
                        }
                        else if ( component_character == 'I' )
                        {
                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "I", COMPRESSION::Discretization, 8, 1.0, 0.0, 0.0, 255.0 )
                                );
                        }
                        else if ( component_character == 'r' )
                        {
                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "R", COMPRESSION::Discretization, 8, 1.0 / 255.0, 0.0, 0.0, 1.0 )
                                );
                        }
                        else if ( component_character == 'R' )
                        {
                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "R", COMPRESSION::Discretization, 8, 1.0, 0.0, 0.0, 255.0 )
                                );
                        }
                        else if ( component_character == 'g' )
                        {
                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "G", COMPRESSION::Discretization, 8, 1.0 / 255.0, 0.0, 0.0, 1.0 )
                                );
                        }
                        else if ( component_character == 'G' )
                        {
                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "G", COMPRESSION::Discretization, 8, 1.0, 0.0, 0.0, 255.0 )
                                );
                        }
                        else if ( component_character == 'b' )
                        {
                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "B", COMPRESSION::Discretization, 8, 1.0 / 255.0, 0.0, 0.0, 1.0 )
                                );
                        }
                        else if ( component_character == 'B' )
                        {
                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "B", COMPRESSION::Discretization, 8, 1.0, 0.0, 0.0, 255.0 )
                                );
                        }
                    }
                }

                CompressedVectorReader
                    compressed_vector_reader
                        = reader.SetUpData3DPointsData(
                              scan_index,
                              BufferPointCount,
                              PointXComponentArray,
                              PointYComponentArray,
                              PointZComponentArray,
                              PointInvalidXyzComponentArray,
                              PointIComponentArray,
                              PointInvalidIComponentArray,
                              PointRComponentArray,
                              PointGComponentArray,
                              PointBComponentArray,
                              PointInvalidRgbComponentArray,
                              PointDComponentArray,
                              PointAComponentArray,
                              PointEComponentArray,
                              PointInvalidDaeComponentArray
                              );

                progress = -1;
                scan_point_index = 0;

                while ( ( point_count = compressed_vector_reader.read() ) > 0 )
                {
                    for ( point_index = 0;
                          point_index < point_count;
                          ++point_index )
                    {
                        if ( scan.IsValidPoint( point_index )
                             && scan_point_index % PointDecimationCount == 0 )
                        {
                            scan.SetPoint( point, point_index, false );
                            TransformPoint( point );

                            pcf_cell
                                = pcf_scan->GetCell(
                                      point.GetComponentValue( x_component_character ),
                                      point.GetComponentValue( y_component_character ),
                                      point.GetComponentValue( z_component_character )
                                      );

                            for ( component_index = 0;
                                  component_index < component_count;
                                  ++component_index )
                            {
                                pcf_cell->AddComponentValue(
                                    pcf_scan->ComponentVector,
                                    component_index,
                                    point.GetComponentValue( output_component_format[ component_index ] )
                                    );
                            }

                            ++( pcf_cell->PointCount );
                        }

                        if ( ProgressOptionIsEnabled )
                        {
                            PrintProgress( progress, scan_point_index, scan.PointCount );
                        }

                        ++scan_point_index;
                    }
                }

                if ( scan_point_index != scan.PointCount )
                {
                    cerr << "*** WARNING : " << scan_point_index << " points (" << scan.PointCount << ")\n";
                }

                pcf_cloud->ScanVector.push_back( pcf_scan );
            }
        }

        cout << "Writing file : " << output_file_path << "\n";

        if ( VerboseOptionIsEnabled )
        {
            pcf_cloud->Dump();
        }

        pcf_cloud->WritePcfFile( output_file_path );
    }
};

// -- FUNCTIONS

int main(
    int argument_count,
    char** argument_array
    )
{
    string
        file_path;
    E57_CLOUD
        cloud;

    VerboseOptionIsEnabled = false;
    ProgressOptionIsEnabled = false;

    --argument_count;
    ++argument_array;

    try
    {
        while ( argument_count > 0
                && argument_array[ 0 ][ 0 ] == '-'
                && argument_array[ 0 ][ 1 ] == '-' )
        {
            if ( argument_count >= 1
                 && !strcmp( argument_array[ 0 ], "--verbose" ) )
            {
                VerboseOptionIsEnabled = true;

                argument_count -= 1;
                argument_array += 1;
            }
            else if ( argument_count >= 1
                 && !strcmp( argument_array[ 0 ], "--progress" ) )
            {
                ProgressOptionIsEnabled = true;

                argument_count -= 1;
                argument_array += 1;
            }
            else if ( argument_count >= 2
                      && !strcmp( argument_array[ 0 ], "--axis-format" )
                      && strlen( argument_array[ 1 ] ) == 3 )
            {
                cloud.AxisFormat = argument_array[ 1 ];

                argument_count -= 2;
                argument_array += 2;
            }
            else if ( argument_count >= 1
                      && !strcmp( argument_array[ 0 ], "--swap-xy" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.SwapsXY = true;

                argument_count -= 1;
                argument_array += 1;
            }
            else if ( argument_count >= 1
                      && !strcmp( argument_array[ 0 ], "--swap-xz" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.SwapsXZ = true;

                argument_count -= 1;
                argument_array += 1;
            }
            else if ( argument_count >= 1
                      && !strcmp( argument_array[ 0 ], "--swap-yz" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.SwapsYZ = true;

                argument_count -= 1;
                argument_array += 1;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--position-offset" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.PositionOffsetVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.PositionOffsetVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.PositionOffsetVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--position-scaling" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.PositionScalingVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.PositionScalingVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.PositionScalingVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--position-rotation" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.PositionRotationVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.PositionRotationVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.PositionRotationVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--position-translation" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.PositionTranslationVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.PositionTranslationVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.PositionTranslationVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--color-offset" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.ColorOffsetVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.ColorOffsetVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.ColorOffsetVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--color-scaling" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.ColorScalingVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.ColorScalingVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.ColorScalingVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--color-translation" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.ColorTranslationVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.ColorTranslationVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.ColorTranslationVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 2
                      && !strcmp( argument_array[ 0 ], "--intensity-offset" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.ColorOffsetVector.W = stof( argument_array[ 1 ] );

                argument_count -= 2;
                argument_array += 2;
            }
            else if ( argument_count >= 2
                      && !strcmp( argument_array[ 0 ], "--intensity-scaling" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.ColorScalingVector.W = stof( argument_array[ 1 ] );

                argument_count -= 2;
                argument_array += 2;
            }
            else if ( argument_count >= 2
                      && !strcmp( argument_array[ 0 ], "--intensity-translation" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.ColorTranslationVector.W = stof( argument_array[ 1 ] );

                argument_count -= 2;
                argument_array += 2;
            }
            else if ( argument_count >= 1
                      && !strcmp( argument_array[ 0 ], "--apply-intensity" ) )
            {
                cloud.PointIsTransformed = true;
                cloud.Transform.AppliesIntensity = true;


                argument_count -= 1;
                argument_array += 1;
            }
            else if ( argument_count >= 2
                      && !strcmp( argument_array[ 0 ], "--ignore-scan" ) )
            {
                cloud.IgnoredScanNameVector.push_back( argument_array[ 1 ] );

                argument_count -= 2;
                argument_array += 2;
            }
            else if ( argument_count >= 2
                      && !strcmp( argument_array[ 0 ], "--select-scan" ) )
            {
                cloud.SelectedScanNameVector.push_back( argument_array[ 1 ] );

                argument_count -= 2;
                argument_array += 2;
            }
            else if ( argument_count >= 2
                      && !strcmp( argument_array[ 0 ], "--point-decimation" ) )
            {
                cloud.PointDecimationCount = stoi( argument_array[ 1 ] );

                argument_count -= 2;
                argument_array += 2;
            }
            else if ( argument_count >= 2
                      && !strcmp( argument_array[ 0 ], "--read-e57-cloud" ) )
            {
                file_path = argument_array[ 1 ];

                argument_count -= 2;
                argument_array += 2;
            }
            else if ( argument_count >= 1
                      && !strcmp( argument_array[ 0 ], "--list-scans" ) )
            {
                cloud.ListScans(
                    file_path
                    );

                argument_count -= 1;
                argument_array += 1;
            }
            else if ( argument_count >= 3
                      && !strcmp( argument_array[ 0 ], "--write-xyz-cloud" ) )
            {
                cloud.WriteXyzFile(
                    file_path,
                    argument_array[ 1 ],
                    argument_array[ 2 ],
                    "xyz"
                    );

                argument_count -= 3;
                argument_array += 3;
            }
            else if ( argument_count >= 3
                      && !strcmp( argument_array[ 0 ], "--write-pts-cloud" ) )
            {
                cloud.WriteXyzFile(
                    file_path,
                    argument_array[ 1 ],
                    argument_array[ 2 ],
                    "pts"
                    );

                argument_count -= 3;
                argument_array += 3;
            }
            else if ( argument_count >= 3
                      && !strcmp( argument_array[ 0 ], "--write-ptx-cloud" ) )
            {
                cloud.WriteXyzFile(
                    file_path,
                    argument_array[ 1 ],
                    argument_array[ 2 ],
                    "ptx"
                    );

                argument_count -= 3;
                argument_array += 3;
            }
            else if ( argument_count >= 5
                      && !strcmp( argument_array[ 0 ], "--write-pcf-cloud" ) )
            {
                cloud.WritePcfFile(
                    file_path,
                    argument_array[ 1 ],
                    argument_array[ 2 ],
                    ( stol( argument_array[ 3 ] ) < 32 ) ? COMPRESSION::Discretization : COMPRESSION::None,
                    ( uint16_t )stol( argument_array[ 3 ] ),
                    stod( argument_array[ 4 ] )
                    );

                argument_count -= 5;
                argument_array += 5;
            }
            else
            {
                break;
            }
        }
    }
    catch( E57Exception & exception_ )
    {
        cerr << "*** ERROR : ";
        exception_.report( __FILE__, __LINE__, __FUNCTION__ );
    }
    catch ( std::exception & exception_ )
    {
        cerr << "*** ERROR : " << exception_.what() << "\n";
    }
    catch ( ... )
    {
        cerr << "*** ERROR\n";
    }

    if ( argument_count == 0 )
    {
        return 0;
    }
    else
    {
        cerr
            << "Usage :\n"
            << "    myst <options>\n"
            << "Options :\n"
            << "    --verbose\n"
            << "    --progress\n"
            << "    --axis-format <component format>\n"
            << "    --swap-xy\n"
            << "    --swap-xz\n"
            << "    --swap-yz\n"
            << "    --apply-intensity\n"
            << "    --position-offset <x> <y> <z>\n"
            << "    --position-scaling <x> <y> <z>\n"
            << "    --position-rotation <x> <y> <z>\n"
            << "    --position-translation <x> <y> <z>\n"
            << "    --color-offset <r> <g> <b> <i>\n"
            << "    --color-scaling <r> <g> <b> <i>\n"
            << "    --color-translation <r> <g> <b> <i>\n"
            << "    --intensity-offset <i>\n"
            << "    --intensity-scaling <i>\n"
            << "    --intensity-translation <i>\n"
            << "    --apply-intensity\n"
            << "    --ignore-scan <scan name>\n"
            << "    --select-scan <scan name>\n"
            << "    --point-decimation <point decimation count>\n"
            << "    --read-e57-cloud <file path>\n"
            << "    --write-xyz-cloud <file path> <component format>\n"
            << "    --write-pts-cloud <file path> <component format>\n"
            << "    --write-ptx-cloud <file path> <component format>\n"
            << "    --write-pcf-cloud <file path> <component format> <position bit count> <position precision>\n";

        return -1;
    }
}

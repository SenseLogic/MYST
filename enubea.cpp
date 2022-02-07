/*
    This file is part of the Enubea distribution.

    https://github.com/senselogic/ENUBEA

    Copyright (C) 2021 Eric Pelzer (ecstatic.coder@gmail.com)

    Enubea is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Enubea is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Enubea.  If not, see <http://www.gnu.org/licenses/>.
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

#include "cell.hpp"
#include "cloud.hpp"
#include "compression.hpp"
#include "link_.hpp"
#include "scan.hpp"
#include "vector_3.hpp"
#include "vector_4.hpp"

using namespace std;
using namespace e57;
using pcf::Abort;
using pcf::GetText;
using pcf::CELL;
using pcf::CLOUD;
using pcf::COMPONENT;
using pcf::COMPRESSION;
using pcf::LINK_;
using pcf::SCAN;
using pcf::VECTOR_3;
using pcf::VECTOR_4;

// -- TYPES

struct TRANSFORM
{
    // -- ATTRIBUTES

    VECTOR_3
        PositionOffsetVector,
        PositionRotationVector,
        PositionScalingVector,
        PositionTranslationVector;
    VECTOR_4
        ColorOffsetVector,
        ColorScalingVector,
        ColorTranslationVector;
    int64_t
        DecimationCount;

    // -- CONSTRUCTORS

    TRANSFORM(
        )
    {
        PositionOffsetVector.SetNull();
        PositionRotationVector.SetNull();
        PositionScalingVector.SetUnit();
        PositionTranslationVector.SetNull();
        ColorOffsetVector.SetNull();
        ColorScalingVector.SetUnit();
        ColorTranslationVector.SetNull();
        DecimationCount = 1;
    }
};

// ~~

struct POINT
{
    // -- ATTRIBUTES

    VECTOR_3
        PositionVector;
    VECTOR_4
        ColorVector;

    // -- CONSTRUCTORS

    POINT(
        )
    {
        PositionVector.SetNull();
        ColorVector.SetNull();
    }

    // -- OPERATORS

    bool operator==(
        const POINT & point
        ) const
    {
        return
            PositionVector == point.PositionVector
            && ColorVector == point.ColorVector;
    }

    // ~~

    bool operator!=(
        const POINT & point
        ) const
    {
        return
            PositionVector != point.PositionVector
            || ColorVector != point.ColorVector;
    }

    // -- INQUIRIES

    void Dump(
        ) const
    {
        cout
            << GetText( PositionVector.X )
            << " "
            << GetText( PositionVector.Y )
            << " "
            << GetText( PositionVector.Z )
            << " "
            << GetText( ColorVector.W )
            << " "
            << GetText( ColorVector.X )
            << " "
            << GetText( ColorVector.Y )
            << " "
            << GetText( ColorVector.Z )
            << "\n";
    }

    // ~~

    double GetComponentValue(
        char component_character
        )
    {
        if ( component_character == 'x' )
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
            return -2048 + ColorVector.W * 4095;
        }
        else if ( component_character == 'i' )
        {
            return ColorVector.W;
        }
        else if ( component_character == 'I' )
        {
            return ColorVector.W * 255.0;
        }
        else if ( component_character == 'r' )
        {
            return ColorVector.X;
        }
        else if ( component_character == 'R' )
        {
            return ColorVector.X * 255.0;
        }
        else if ( component_character == 'g' )
        {
            return ColorVector.Y;
        }
        else if ( component_character == 'G' )
        {
            return ColorVector.Y * 255.0;
        }
        else if ( component_character == 'b' )
        {
            return ColorVector.Z;
        }
        else if ( component_character == 'B' )
        {
            return ColorVector.Z * 255.0;
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
    Data3D
        Data;
    int64_t
        RowCount,
        ColumnCount,
        PointCount,
        GroupCount,
        MaximumPointCount;
    bool
        IsColumnIndex;
};

// ~~

struct E57_CLOUD
{
    // -- ATTRIBUTES

    bool
        HasTransform,
        IsVerbose;
    TRANSFORM
        Transform;
    vector<E57_SCAN>
        ScanVector;

    // -- CONSTRUCTORS

    E57_CLOUD(
        ) :
        HasTransform( false ),
        IsVerbose( false ),
        Transform(),
        ScanVector()
    {
    }

    // -- INQUIRIES

    string GetComponentName(
        char component_character
        )
    {
        if ( component_character == 'x'
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

    // -- OPERATIONS

    void TransformPoint(
        POINT & point
        )
    {
        if ( IsVerbose )
        {
            point.Dump();
        }

        if ( HasTransform )
        {
            point = point.GetTransformedPoint( Transform );

            if ( IsVerbose )
            {
                cout << "=> ";
                point.Dump();
            }
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
        const int64_t
            MaximumPointCount = 65536;
        static double
            point_i_array[ MaximumPointCount ],
            point_x_array[ MaximumPointCount ],
            point_y_array[ MaximumPointCount ],
            point_z_array[ MaximumPointCount ];
        static uint16_t
            point_b_array[ MaximumPointCount ],
            point_g_array[ MaximumPointCount ],
            point_r_array[ MaximumPointCount ];
        bool
            point_has_b_field,
            point_has_g_field,
            point_has_i_field,
            point_has_r_field,
            point_has_x_field,
            point_has_y_field,
            point_has_z_field;
        double
            minimum_b,
            maximum_b,
            minimum_g,
            maximum_g,
            minimum_i,
            maximum_i,
            minimum_r,
            maximum_r,
            minimum_x,
            maximum_x,
            minimum_y,
            maximum_y,
            minimum_z,
            maximum_z;
        int32_t
            scan_count,
            scan_index;
        int64_t
            point_count,
            point_index;
        uint64_t
            component_count,
            component_index;
        ofstream
            output_file_stream;
        POINT
            point;
        E57_SCAN
            * scan;
        VECTOR_3
            x_axis_vector,
            y_axis_vector,
            z_axis_vector;

        component_count = output_component_format.size();

        cout << "Reading file : " << input_file_path << "\n";

        Reader
            file_reader( input_file_path );

        scan_count = file_reader.GetData3DCount();
        ScanVector.resize( scan_count );

        point_count = 0;

        for ( scan_index = 0;
              scan_index < scan_count;
              ++scan_index )
        {
            scan = &ScanVector[ scan_index ];
            file_reader.ReadData3D( scan_index, scan->Data );

            scan->Name = scan->Data.name;
            scan->PositionVector.X = scan->Data.pose.translation.x;
            scan->PositionVector.Y = scan->Data.pose.translation.y;
            scan->PositionVector.Z = scan->Data.pose.translation.z;
            scan->RotationVector.X = scan->Data.pose.rotation.x;
            scan->RotationVector.Y = scan->Data.pose.rotation.y;
            scan->RotationVector.Z = scan->Data.pose.rotation.z;
            scan->RotationVector.W = scan->Data.pose.rotation.w;

            file_reader.GetData3DSizes(
                scan_index,
                scan->RowCount,
                scan->ColumnCount,
                scan->PointCount,
                scan->GroupCount,
                scan->MaximumPointCount,
                scan->IsColumnIndex
                );

            point_count += scan->PointCount;
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

            if ( output_file_format == "ptx" )
            {
                scan->RotationVector.GetAxisVectors(
                    x_axis_vector,
                    y_axis_vector,
                    z_axis_vector
                    );

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
                    << x_axis_vector.X
                    << " "
                    << x_axis_vector.Y
                    << " "
                    << x_axis_vector.Z
                    << "\n"
                    << y_axis_vector.X
                    << " "
                    << y_axis_vector.Y
                    << " "
                    << y_axis_vector.Z
                    << "\n"
                    << z_axis_vector.X
                    << " "
                    << z_axis_vector.Y
                    << " "
                    << z_axis_vector.Z
                    << "\n"
                    << x_axis_vector.X
                    << " "
                    << x_axis_vector.Y
                    << " "
                    << x_axis_vector.Z
                    << " 0\n"
                    << y_axis_vector.X
                    << " "
                    << y_axis_vector.Y
                    << " "
                    << y_axis_vector.Z
                    << " 0\n"
                    << z_axis_vector.X
                    << " "
                    << z_axis_vector.Y
                    << " "
                    << z_axis_vector.Z
                    << " 0\n"
                    << scan->PositionVector.X
                    << " "
                    << scan->PositionVector.Y
                    << " "
                    << scan->PositionVector.Z
                    << " 1\n";
            }

            point_has_x_field = scan->Data.pointFields.cartesianXField;
            point_has_y_field = scan->Data.pointFields.cartesianYField;
            point_has_z_field = scan->Data.pointFields.cartesianZField;
            point_has_i_field = scan->Data.pointFields.intensityField;
            point_has_r_field = scan->Data.pointFields.colorRedField;
            point_has_g_field = scan->Data.pointFields.colorGreenField;
            point_has_b_field = scan->Data.pointFields.colorBlueField;

            minimum_x = scan->Data.cartesianBounds.xMinimum;
            maximum_x = scan->Data.cartesianBounds.xMaximum;
            minimum_y = scan->Data.cartesianBounds.yMinimum;
            maximum_y = scan->Data.cartesianBounds.yMaximum;
            minimum_z = scan->Data.cartesianBounds.zMinimum;
            maximum_z = scan->Data.cartesianBounds.zMaximum;
            minimum_i = scan->Data.intensityLimits.intensityMinimum;
            maximum_i = scan->Data.intensityLimits.intensityMaximum;
            minimum_r = scan->Data.colorLimits.colorRedMinimum;
            maximum_r = scan->Data.colorLimits.colorRedMaximum;
            minimum_g = scan->Data.colorLimits.colorGreenMinimum;
            maximum_g = scan->Data.colorLimits.colorGreenMaximum;
            minimum_b = scan->Data.colorLimits.colorBlueMinimum;
            maximum_b = scan->Data.colorLimits.colorBlueMaximum;

            if ( IsVerbose )
            {
                if ( point_has_x_field )
                {
                    cout << "X : " << GetText( minimum_x ) << " / " << GetText( maximum_x ) << "\n";
                }

                if ( point_has_y_field )
                {
                    cout << "Y : " << GetText( minimum_y ) << " / " << GetText( maximum_y ) << "\n";
                }

                if ( point_has_z_field )
                {
                    cout << "Z : " << GetText( minimum_z ) << " / " << GetText( maximum_z ) << "\n";
                }

                if ( point_has_i_field )
                {
                    cout << "I : " << GetText( minimum_i ) << " / " << GetText( maximum_i ) << "\n";
                }

                if ( point_has_r_field )
                {
                    cout << "R : " << GetText( minimum_r ) << " / " << GetText( maximum_r ) << "\n";
                }

                if ( point_has_g_field )
                {
                    cout << "G : " << GetText( minimum_g ) << " / " << GetText( maximum_g ) << "\n";
                }

                if ( point_has_b_field )
                {
                    cout << "B : " << GetText( minimum_b ) << " / " << GetText( maximum_b ) << "\n";
                }
            }

            if ( point_has_x_field
                 && point_has_y_field
                 && point_has_z_field )
            {
                CompressedVectorReader
                    compressed_vector_reader
                        = file_reader.SetUpData3DPointsData(
                            scan_index,
                            MaximumPointCount,
                            point_x_array,
                            point_y_array,
                            point_z_array,
                            nullptr,
                            point_i_array,
                            nullptr,
                            point_r_array,
                            point_g_array,
                            point_b_array,
                            nullptr,
                            nullptr,
                            nullptr,
                            nullptr,
                            nullptr,
                            nullptr
                            );

                while ( ( point_count = compressed_vector_reader.read() ) > 0 )
                {
                    for ( point_index = 0;
                          point_index < point_count;
                          ++point_index )
                    {
                        point.PositionVector.X = point_x_array[ point_index ];
                        point.PositionVector.Y = point_y_array[ point_index ];
                        point.PositionVector.Z = point_z_array[ point_index ];

                        if ( point_has_r_field )
                        {
                            point.ColorVector.X = ( point_r_array[ point_index ] - minimum_r ) / ( maximum_r - minimum_r );
                        }

                        if ( point_has_g_field )
                        {
                            point.ColorVector.Y = ( point_g_array[ point_index ] - minimum_g ) / ( maximum_g - minimum_g );
                        }

                        if ( point_has_b_field )
                        {
                            point.ColorVector.Z = ( point_b_array[ point_index ] - minimum_b ) / ( maximum_b - minimum_b );
                        }

                        if ( point_has_i_field )
                        {
                            point.ColorVector.W = ( point_i_array[ point_index ] - minimum_i ) / ( maximum_i - minimum_i );
                        }

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
        const int64_t
            MaximumPointCount = 65536;
        static double
            point_i_array[ MaximumPointCount ],
            point_x_array[ MaximumPointCount ],
            point_y_array[ MaximumPointCount ],
            point_z_array[ MaximumPointCount ];
        static uint16_t
            point_b_array[ MaximumPointCount ],
            point_g_array[ MaximumPointCount ],
            point_r_array[ MaximumPointCount ];
        bool
            point_has_b_field,
            point_has_g_field,
            point_has_i_field,
            point_has_r_field,
            point_has_x_field,
            point_has_y_field,
            point_has_z_field;
        char
            component_character,
            x_component_character,
            y_component_character,
            z_component_character;
        double
            minimum_b,
            maximum_b,
            minimum_g,
            maximum_g,
            minimum_i,
            maximum_i,
            minimum_r,
            maximum_r,
            minimum_x,
            maximum_x,
            minimum_y,
            maximum_y,
            minimum_z,
            maximum_z;
        int32_t
            scan_count,
            scan_index;
        int64_t
            point_count,
            point_index;
        uint64_t
            component_count,
            component_index;
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

        Reader
            file_reader( input_file_path );

        component_count = output_component_format.size();

        scan_count = file_reader.GetData3DCount();
        ScanVector.resize( scan_count );

        for ( scan_index = 0;
              scan_index < scan_count;
              ++scan_index )
        {
            file_reader.ReadData3D( scan_index, scan.Data );

            scan.Name = scan.Data.name;
            scan.PositionVector.X = scan.Data.pose.translation.x;
            scan.PositionVector.Y = scan.Data.pose.translation.y;
            scan.PositionVector.Z = scan.Data.pose.translation.z;
            scan.RotationVector.X = scan.Data.pose.rotation.x;
            scan.RotationVector.Y = scan.Data.pose.rotation.y;
            scan.RotationVector.Z = scan.Data.pose.rotation.z;
            scan.RotationVector.W = scan.Data.pose.rotation.w;

            file_reader.GetData3DSizes(
                scan_index,
                scan.RowCount,
                scan.ColumnCount,
                scan.PointCount,
                scan.GroupCount,
                scan.MaximumPointCount,
                scan.IsColumnIndex
                );

            point_has_x_field = scan.Data.pointFields.cartesianXField;
            point_has_y_field = scan.Data.pointFields.cartesianYField;
            point_has_z_field = scan.Data.pointFields.cartesianZField;
            point_has_i_field = scan.Data.pointFields.intensityField;
            point_has_r_field = scan.Data.pointFields.colorRedField;
            point_has_g_field = scan.Data.pointFields.colorGreenField;
            point_has_b_field = scan.Data.pointFields.colorBlueField;

            minimum_x = scan.Data.cartesianBounds.xMinimum;
            maximum_x = scan.Data.cartesianBounds.xMaximum;
            minimum_y = scan.Data.cartesianBounds.yMinimum;
            maximum_y = scan.Data.cartesianBounds.yMaximum;
            minimum_z = scan.Data.cartesianBounds.zMinimum;
            maximum_z = scan.Data.cartesianBounds.zMaximum;
            minimum_i = scan.Data.intensityLimits.intensityMinimum;
            maximum_i = scan.Data.intensityLimits.intensityMaximum;
            minimum_r = scan.Data.colorLimits.colorRedMinimum;
            maximum_r = scan.Data.colorLimits.colorRedMaximum;
            minimum_g = scan.Data.colorLimits.colorGreenMinimum;
            maximum_g = scan.Data.colorLimits.colorGreenMaximum;
            minimum_b = scan.Data.colorLimits.colorBlueMinimum;
            maximum_b = scan.Data.colorLimits.colorBlueMaximum;

            if ( IsVerbose )
            {
                if ( point_has_x_field )
                {
                    cout << "X : " << GetText( minimum_x ) << " / " << GetText( maximum_x ) << "\n";
                }

                if ( point_has_y_field )
                {
                    cout << "Y : " << GetText( minimum_y ) << " / " << GetText( maximum_y ) << "\n";
                }

                if ( point_has_z_field )
                {
                    cout << "Z : " << GetText( minimum_z ) << " / " << GetText( maximum_z ) << "\n";
                }

                if ( point_has_i_field )
                {
                    cout << "I : " << GetText( minimum_i ) << " / " << GetText( maximum_i ) << "\n";
                }

                if ( point_has_r_field )
                {
                    cout << "R : " << GetText( minimum_r ) << " / " << GetText( maximum_r ) << "\n";
                }

                if ( point_has_g_field )
                {
                    cout << "G : " << GetText( minimum_g ) << " / " << GetText( maximum_g ) << "\n";
                }

                if ( point_has_b_field )
                {
                    cout << "B : " << GetText( minimum_b ) << " / " << GetText( maximum_b ) << "\n";
                }
            }

            if ( point_has_x_field
                 && point_has_y_field
                 && point_has_z_field )
            {
                pcf_scan = new SCAN();
                pcf_scan->Name = scan.Name;
                pcf_scan->PointCount = scan.PointCount;
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
                                new COMPONENT( "X", COMPRESSION::Discretization, position_bit_count, position_precision, 0.0, minimum_x, maximum_x )
                                );
                        }
                        else if ( component_character == 'y'
                                  || component_character == 'Y' )
                        {
                            y_component_character = component_character;

                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "Y", COMPRESSION::Discretization, position_bit_count, position_precision, 0.0, minimum_y, maximum_y )
                                );
                        }
                        else if ( component_character == 'z'
                                  || component_character == 'Z' )
                        {
                            z_component_character = component_character;

                            pcf_scan->ComponentVector.push_back(
                                new COMPONENT( "Z", COMPRESSION::Discretization, position_bit_count, position_precision, 0.0, minimum_z, maximum_z )
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
                        = file_reader.SetUpData3DPointsData(
                            scan_index,
                            MaximumPointCount,
                            point_x_array,
                            point_y_array,
                            point_z_array,
                            nullptr,
                            point_i_array,
                            nullptr,
                            point_r_array,
                            point_g_array,
                            point_b_array,
                            nullptr,
                            nullptr,
                            nullptr,
                            nullptr,
                            nullptr,
                            nullptr
                            );

                while ( ( point_count = compressed_vector_reader.read() ) > 0 )
                {
                    for ( point_index = 0;
                          point_index < point_count;
                          ++point_index )
                    {
                        point.PositionVector.X = point_x_array[ point_index ];
                        point.PositionVector.Y = point_y_array[ point_index ];
                        point.PositionVector.Z = point_z_array[ point_index ];

                        if ( point_has_i_field )
                        {
                            point.ColorVector.W = ( point_i_array[ point_index ] - minimum_i ) / ( maximum_i - minimum_i );
                        }

                        if ( point_has_r_field )
                        {
                            point.ColorVector.X = ( point_r_array[ point_index ] - minimum_r ) / ( maximum_r - minimum_r );
                        }

                        if ( point_has_g_field )
                        {
                            point.ColorVector.Y = ( point_g_array[ point_index ] - minimum_g ) / ( maximum_g - minimum_g );
                        }

                        if ( point_has_b_field )
                        {
                            point.ColorVector.Z = ( point_b_array[ point_index ] - minimum_b ) / ( maximum_b - minimum_b );
                        }

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
                }

                pcf_cloud->ScanVector.push_back( pcf_scan );
            }
        }

        cout << "Writing file : " << output_file_path << "\n";

        if ( IsVerbose )
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
                cloud.IsVerbose = true;

                argument_count -= 1;
                argument_array += 1;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--position-offset" ) )
            {
                cloud.HasTransform = true;
                cloud.Transform.PositionOffsetVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.PositionOffsetVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.PositionOffsetVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--position-scaling" ) )
            {
                cloud.HasTransform = true;
                cloud.Transform.PositionScalingVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.PositionScalingVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.PositionScalingVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--position-rotation" ) )
            {
                cloud.HasTransform = true;
                cloud.Transform.PositionRotationVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.PositionRotationVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.PositionRotationVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--position-translation" ) )
            {
                cloud.HasTransform = true;
                cloud.Transform.PositionTranslationVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.PositionTranslationVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.PositionTranslationVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 5
                      && !strcmp( argument_array[ 0 ], "--color-offset" ) )
            {
                cloud.HasTransform = true;
                cloud.Transform.ColorOffsetVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.ColorOffsetVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.ColorOffsetVector.Z = stof( argument_array[ 3 ] );
                cloud.Transform.ColorOffsetVector.W = stof( argument_array[ 4 ] );

                argument_count -= 5;
                argument_array += 5;
            }
            else if ( argument_count >= 2
                      && !strcmp( argument_array[ 0 ], "--color-scaling" ) )
            {
                cloud.HasTransform = true;
                cloud.Transform.ColorScalingVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.ColorScalingVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.ColorScalingVector.Z = stof( argument_array[ 3 ] );
                cloud.Transform.ColorScalingVector.W = stof( argument_array[ 4 ] );

                argument_count -= 5;
                argument_array += 5;
            }
            else if ( argument_count >= 2
                      && !strcmp( argument_array[ 0 ], "--color-translation" ) )
            {
                cloud.HasTransform = true;
                cloud.Transform.ColorTranslationVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.ColorTranslationVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.ColorTranslationVector.Z = stof( argument_array[ 3 ] );
                cloud.Transform.ColorTranslationVector.W = stof( argument_array[ 4 ] );

                argument_count -= 5;
                argument_array += 5;
            }
            else if ( argument_count >= 2
                      && !strcmp( argument_array[ 0 ], "--decimation-count" ) )
            {
                cloud.Transform.DecimationCount = stoi( argument_array[ 1 ] );

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
        cerr
            << "Error :\n";

        exception_.report( __FILE__, __LINE__, __FUNCTION__ );
    }
    catch ( std::exception & exception_ )
    {
        cerr
            << "Error :\n"
            << exception_.what() << "\n";
    }
    catch ( ... )
    {
        cerr
            << "Error.\n";
    }

    if ( argument_count == 0 )
    {
        return 0;
    }
    else
    {
        cerr
            << "Usage :\n"
            << "    enubea <options>\n"
            << "Options :\n"
            << "    --verbose\n"
            << "    --position-offset <x> <y> <z>\n"
            << "    --position-scaling <x> <y> <z>\n"
            << "    --position-rotation <x> <y> <z>\n"
            << "    --position-translation <x> <y> <z>\n"
            << "    --color-offset <r> <g> <b> <i>\n"
            << "    --color-scaling <r> <g> <b> <i>\n"
            << "    --color-translation <r> <g> <b> <i>\n"
            << "    --decimation-count <decimation count>\n"
            << "    --read-e57-cloud <file path>\n"
            << "    --write-xyz-cloud <file path> <component format>\n"
            << "    --write-pts-cloud <file path> <component format>\n"
            << "    --write-ptx-cloud <file path> <component format>\n"
            << "    --write-pcf-cloud <file path> <component format> <position bit count> <position precision>\n";

        return -1;
    }
}

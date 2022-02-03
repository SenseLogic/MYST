/*
    This file is part of the Xenius distribution.

    https://github.com/senselogic/XENIUS

    Copyright (C) 2021 Eric Pelzer (ecstatic.coder@gmail.com)

    Xenius is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Xenius is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Xenius.  If not, see <http://www.gnu.org/licenses/>.
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

    // -- INQUIRIES

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

    TRANSFORM
        Transform;
    vector<E57_SCAN>
        ScanVector;

    // -- OPERATIONS

    void WriteXyzOrPtsFile(
        const string & input_file_path,
        const string & output_file_path,
        const string & output_line_format,
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
            point_has_xyz_fields,
            point_has_y_field,
            point_has_z_field;
        char
            field_character;
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
            field_count,
            field_index,
            point_count,
            point_index;
        ofstream
            output_file_stream;
        POINT
            point;
        E57_SCAN
            * scan;

        cout
            << "Reading file : " << input_file_path << "\n";

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

        cout
            << "Writing file : " << output_file_path << "\n";

        output_file_stream.open( output_file_path );

        if ( output_file_format == "pts" )
        {
            output_file_stream << point_count << "\n";
        }

        field_count = output_line_format.size();

        for ( scan_index = 0;
              scan_index < scan_count;
              ++scan_index )
        {
            scan = &ScanVector[ scan_index ];

            point_has_x_field = scan->Data.pointFields.cartesianXField;
            point_has_y_field = scan->Data.pointFields.cartesianYField;
            point_has_z_field = scan->Data.pointFields.cartesianZField;
            point_has_r_field = scan->Data.pointFields.colorRedField;
            point_has_g_field = scan->Data.pointFields.colorGreenField;
            point_has_b_field = scan->Data.pointFields.colorBlueField;
            point_has_i_field = scan->Data.pointFields.intensityField;

            point_has_xyz_fields = point_has_x_field && point_has_y_field && point_has_z_field;

            minimum_x = scan->Data.cartesianBounds.xMinimum;
            maximum_x = scan->Data.cartesianBounds.xMaximum;
            minimum_y = scan->Data.cartesianBounds.yMinimum;
            maximum_y = scan->Data.cartesianBounds.yMaximum;
            minimum_z = scan->Data.cartesianBounds.zMinimum;
            maximum_z = scan->Data.cartesianBounds.zMaximum;
            minimum_r = scan->Data.colorLimits.colorRedMinimum;
            maximum_r = scan->Data.colorLimits.colorRedMaximum;
            minimum_g = scan->Data.colorLimits.colorGreenMinimum;
            maximum_g = scan->Data.colorLimits.colorGreenMaximum;
            minimum_b = scan->Data.colorLimits.colorBlueMinimum;
            maximum_b = scan->Data.colorLimits.colorBlueMaximum;
            minimum_i = scan->Data.intensityLimits.intensityMinimum;
            maximum_i = scan->Data.intensityLimits.intensityMaximum;

            if ( point_has_xyz_fields )
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
                            point.ColorVector.X = floor( ( point_r_array[ point_index ] - minimum_r ) * 255 / ( maximum_r - minimum_r ) );
                        }

                        if ( point_has_g_field )
                        {
                            point.ColorVector.Y = floor( ( point_g_array[ point_index ] - minimum_g ) * 255 / ( maximum_g - minimum_g ) );
                        }

                        if ( point_has_b_field )
                        {
                            point.ColorVector.Z = floor( ( point_b_array[ point_index ] - minimum_b ) * 255 / ( maximum_b - minimum_b ) );
                        }

                        if ( point_has_i_field )
                        {
                            point.ColorVector.W = floor( ( point_i_array[ point_index ] - minimum_i ) * 255 / ( maximum_i - minimum_i ) );
                        }

                        point = point.GetTransformedPoint( Transform );

                        for ( field_index = 0;
                              field_index < field_count;
                              ++field_index )
                        {
                            if ( field_index > 0 )
                            {
                                output_file_stream << " ";
                            }

                            field_character = output_line_format[ field_index ];

                            if ( field_character == 'x' )
                            {
                                output_file_stream << ( -point.PositionVector.X );
                            }
                            else if ( field_character == 'y' )
                            {
                                output_file_stream << ( -point.PositionVector.Y );
                            }
                            else if ( field_character == 'z' )
                            {
                                output_file_stream << ( -point.PositionVector.Z );
                            }
                            else if ( field_character == 'X' )
                            {
                                output_file_stream << point.PositionVector.X;
                            }
                            else if ( field_character == 'Y' )
                            {
                                output_file_stream << point.PositionVector.Y;
                            }
                            else if ( field_character == 'Z' )
                            {
                                output_file_stream << point.PositionVector.Z;
                            }
                            else if ( field_character == 'R' )
                            {
                                output_file_stream << point.ColorVector.X;
                            }
                            else if ( field_character == 'G' )
                            {
                                output_file_stream << point.ColorVector.Y;
                            }
                            else if ( field_character == 'B' )
                            {
                                output_file_stream << point.ColorVector.Z;
                            }
                            else if ( field_character == 'I' )
                            {
                                output_file_stream << point.ColorVector.W;
                            }
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
            point_has_rgb_fields,
            point_has_x_field,
            point_has_xyz_fields,
            point_has_y_field,
            point_has_z_field;
        double
            color_maximum,
            color_minimum,
            color_precision,
            intensity_maximum,
            intensity_minimum,
            intensity_precision,
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
            maximum_z,
            position_maximum,
            position_minimum;
        int32_t
            scan_count,
            scan_index;
        int64_t
            point_count,
            point_index;
        uint16_t
            color_bit_count,
            intensity_bit_count;
        uint64_t
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

        position_minimum = 0.0;
        position_maximum = 0.0;
        intensity_bit_count = 12;
        intensity_precision = 1.0;
        intensity_minimum = -2048.0;
        intensity_maximum = 2047.0;
        color_bit_count = 8;
        color_precision = 1.0 / 255.0;
        color_minimum = 0.0;
        color_maximum = 255.0;

        cout
            << "Reading file : " << input_file_path << "\n";

        pcf_cloud = new pcf::CLOUD();

        Reader
            file_reader( input_file_path );

        scan_count = file_reader.GetData3DCount();
        ScanVector.resize( scan_count );

        for ( scan_index = 0;
              scan_index < scan_count;
              ++scan_index )
        {
            file_reader.ReadData3D( scan_index, scan.Data );

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
            point_has_r_field = scan.Data.pointFields.colorRedField;
            point_has_g_field = scan.Data.pointFields.colorGreenField;
            point_has_b_field = scan.Data.pointFields.colorBlueField;
            point_has_i_field = scan.Data.pointFields.intensityField;

            point_has_xyz_fields = point_has_x_field && point_has_y_field && point_has_z_field;
            point_has_rgb_fields = point_has_r_field && point_has_g_field && point_has_b_field;

            minimum_x = scan.Data.cartesianBounds.xMinimum;
            maximum_x = scan.Data.cartesianBounds.xMaximum;
            minimum_y = scan.Data.cartesianBounds.yMinimum;
            maximum_y = scan.Data.cartesianBounds.yMaximum;
            minimum_z = scan.Data.cartesianBounds.zMinimum;
            maximum_z = scan.Data.cartesianBounds.zMaximum;
            minimum_r = scan.Data.colorLimits.colorRedMinimum;
            maximum_r = scan.Data.colorLimits.colorRedMaximum;
            minimum_g = scan.Data.colorLimits.colorGreenMinimum;
            maximum_g = scan.Data.colorLimits.colorGreenMaximum;
            minimum_b = scan.Data.colorLimits.colorBlueMinimum;
            maximum_b = scan.Data.colorLimits.colorBlueMaximum;
            minimum_i = scan.Data.intensityLimits.intensityMinimum;
            maximum_i = scan.Data.intensityLimits.intensityMaximum;

            if ( point_has_x_field
                 && point_has_y_field
                 && point_has_z_field )
            {
                pcf_scan = new SCAN();

                if ( scan_index == 0 )
                {
                    if ( compression == COMPRESSION::None )
                    {
                        pcf_cloud->ComponentVector.push_back( new COMPONENT( "X", COMPRESSION::None, 32, 0.0, 0.0 ) );
                        pcf_cloud->ComponentVector.push_back( new COMPONENT( "Y", COMPRESSION::None, 32, 0.0, 0.0 ) );
                        pcf_cloud->ComponentVector.push_back( new COMPONENT( "Z", COMPRESSION::None, 32, 0.0, 0.0 ) );

                        if ( point_has_i_field )
                        {
                            pcf_cloud->ComponentVector.push_back( new COMPONENT( "I", COMPRESSION::None, 32, 0.0, 0.0 ) );
                        }

                        if ( point_has_rgb_fields )
                        {
                            pcf_cloud->ComponentVector.push_back( new COMPONENT( "R", COMPRESSION::None, 32, 0.0, 0.0 ) );
                            pcf_cloud->ComponentVector.push_back( new COMPONENT( "G", COMPRESSION::None, 32, 0.0, 0.0 ) );
                            pcf_cloud->ComponentVector.push_back( new COMPONENT( "B", COMPRESSION::None, 32, 0.0, 0.0 ) );
                        }
                    }
                    else
                    {
                        assert( compression == COMPRESSION::Discretization );

                        pcf_cloud->ComponentVector.push_back( new COMPONENT( "X", COMPRESSION::Discretization, position_bit_count, position_precision, position_minimum, position_maximum ) );
                        pcf_cloud->ComponentVector.push_back( new COMPONENT( "Y", COMPRESSION::Discretization, position_bit_count, position_precision, position_minimum, position_maximum ) );
                        pcf_cloud->ComponentVector.push_back( new COMPONENT( "Z", COMPRESSION::Discretization, position_bit_count, position_precision, position_minimum, position_maximum ) );

                        if ( point_has_i_field )
                        {
                            pcf_cloud->ComponentVector.push_back( new COMPONENT( "I", COMPRESSION::Discretization, intensity_bit_count, intensity_precision, intensity_minimum, intensity_maximum ) );
                        }

                        if ( point_has_rgb_fields )
                        {
                            pcf_cloud->ComponentVector.push_back( new COMPONENT( "R", COMPRESSION::Discretization, color_bit_count, color_precision, color_minimum, color_maximum ) );
                            pcf_cloud->ComponentVector.push_back( new COMPONENT( "G", COMPRESSION::Discretization, color_bit_count, color_precision, color_minimum, color_maximum ) );
                            pcf_cloud->ComponentVector.push_back( new COMPONENT( "B", COMPRESSION::Discretization, color_bit_count, color_precision, color_minimum, color_maximum ) );
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
                    pcf_scan->PointCount = point_count;
                    pcf_scan->ColumnCount = point_count;
                    pcf_scan->RowCount = 1;

                    for ( point_index = 0;
                          point_index < point_count;
                          ++point_index )
                    {
                        point.PositionVector.X = point_x_array[ point_index ];
                        point.PositionVector.Y = point_y_array[ point_index ];
                        point.PositionVector.Z = point_z_array[ point_index ];

                        if ( point_has_i_field )
                        {
                            point.ColorVector.W = floor( ( point_i_array[ point_index ] - minimum_i ) * 255 / ( maximum_i - minimum_i ) );
                        }

                        if ( point_has_rgb_fields )
                        {
                            point.ColorVector.X = floor( ( point_r_array[ point_index ] - minimum_r ) * 255 / ( maximum_r - minimum_r ) );
                            point.ColorVector.Y = floor( ( point_g_array[ point_index ] - minimum_g ) * 255 / ( maximum_g - minimum_g ) );
                            point.ColorVector.Z = floor( ( point_b_array[ point_index ] - minimum_b ) * 255 / ( maximum_b - minimum_b ) );
                        }

                        point = point.GetTransformedPoint( Transform );

                        pcf_cell = pcf_scan->GetCell( pcf_cloud->ComponentVector, point.PositionVector.X, point.PositionVector.Y, point.PositionVector.Z );
                        pcf_cell->AddComponentValue( pcf_cloud->ComponentVector, 0, point.PositionVector.X );
                        pcf_cell->AddComponentValue( pcf_cloud->ComponentVector, 1, point.PositionVector.Y );
                        pcf_cell->AddComponentValue( pcf_cloud->ComponentVector, 2, point.PositionVector.Z );
                        component_index = 3;

                        if ( point_has_i_field )
                        {
                            pcf_cell->AddComponentValue( pcf_cloud->ComponentVector, component_index, point.ColorVector.W );
                            ++component_index;
                        }

                        if ( point_has_rgb_fields )
                        {
                            pcf_cell->AddComponentValue( pcf_cloud->ComponentVector, component_index, point.ColorVector.X );
                            pcf_cell->AddComponentValue( pcf_cloud->ComponentVector, component_index + 1, point.ColorVector.Y );
                            pcf_cell->AddComponentValue( pcf_cloud->ComponentVector, component_index + 2, point.ColorVector.Z );
                        }

                        ++( pcf_cell->PointCount );
                    }
                }

                pcf_cloud->ScanVector.push_back( pcf_scan );
            }
        }

        cout
            << "Writing file : " << output_file_path << "\n";

    }
};

// -- FUNCTIONS

bool HasSuffix(
    const string & text,
    const string & suffix
    )
{
    return
        text.length() >= suffix.length()
        && !text.compare( text.length() - suffix.length(), suffix.length(), suffix );
}

// ~~

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
            if ( argument_count >= 4
                 && !strcmp( argument_array[ 0 ], "--position-offset" ) )
            {
                cloud.Transform.PositionOffsetVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.PositionOffsetVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.PositionOffsetVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--position-scaling" ) )
            {
                cloud.Transform.PositionScalingVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.PositionScalingVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.PositionScalingVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--position-rotation" ) )
            {
                cloud.Transform.PositionRotationVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.PositionRotationVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.PositionRotationVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--position-translation" ) )
            {
                cloud.Transform.PositionTranslationVector.X = stof( argument_array[ 1 ] );
                cloud.Transform.PositionTranslationVector.Y = stof( argument_array[ 2 ] );
                cloud.Transform.PositionTranslationVector.Z = stof( argument_array[ 3 ] );

                argument_count -= 4;
                argument_array += 4;
            }
            else if ( argument_count >= 5
                      && !strcmp( argument_array[ 0 ], "--color-offset" ) )
            {
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
                cloud.WriteXyzOrPtsFile(
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
                cloud.WriteXyzOrPtsFile(
                    file_path,
                    argument_array[ 1 ],
                    argument_array[ 2 ],
                    "pts"
                    );

                argument_count -= 3;
                argument_array += 3;
            }
            else if ( argument_count >= 4
                      && !strcmp( argument_array[ 0 ], "--write-pcf-cloud" ) )
            {
                cloud.WritePcfFile(
                    file_path,
                    argument_array[ 1 ],
                    ( COMPRESSION )stol( argument_array[ 2 ] ),
                    ( uint16_t )stol( argument_array[ 3 ] ),
                    stod( argument_array[ 4 ] )
                    );

                argument_count -= 3;
                argument_array += 3;
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
            << "    xenius <options>\n"
            << "Options :\n"
            << "    --position-offset <x> <y> <z>\n"
            << "    --position-scaling <x> <y> <z>\n"
            << "    --position-rotation <x> <y> <z>\n"
            << "    --position-translation <x> <y> <z>\n"
            << "    --color-offset <r> <g> <b> <i>\n"
            << "    --color-scaling <r> <g> <b> <i>\n"
            << "    --color-translation <r> <g> <b> <i>\n"
            << "    --decimation-count <decimation count>\n"
            << "    --read-e57-cloud <file path>\n"
            << "    --write-xyz-cloud <file path> <line format>\n"
            << "    --write-pts-cloud <file path> <line format>\n"
            << "    --write-pcf-cloud <file path> <compression> <position bit count> <position precision>\n";

        return -1;
    }
}

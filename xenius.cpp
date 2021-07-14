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

using namespace e57;
using namespace std;

// -- TYPES

struct VECTOR_3
{
    // -- ATTRIBUTES

    double
        X,
        Y,
        Z;

    // -- OPERATIONS

    void SetNull(
        )
    {
        X = 0.0;
        Y = 0.0;
        Z = 0.0;
    }

    // ~~

    void SetUnit(
        )
    {
        X = 1.0;
        Y = 1.0;
        Z = 1.0;
    }

    // ~~

    void AddVector(
        const VECTOR_3 & vector
        )
    {
        X += vector.X;
        Y += vector.Y;
        Z += vector.Z;
    }

    // ~~

    void AddScaledVector(
        const VECTOR_3 & vector,
        double factor
        )
    {
        X += vector.X * factor;
        Y += vector.Y * factor;
        Z += vector.Z * factor;
    }

    // ~~

    void MultiplyScalar(
        double scalar
        )
    {
        X *= scalar;
        Y *= scalar;
        Z *= scalar;
    }

    // ~~

    void Translate(
        double x_translation,
        double y_translation,
        double z_translation
        )
    {
        X += x_translation;
        Y += y_translation;
        Z += z_translation;
    }

    // ~~

    void Scale(
        double x_scaling,
        double y_scaling,
        double z_scaling
        )
    {
        X *= x_scaling;
        Y *= y_scaling;
        Z *= z_scaling;
    }

    // ~~

    void RotateAroundX(
        double x_angle_cosinus,
        double x_angle_sinus
        )
    {
        double
            y;

        y = Y;
        Y = Y * x_angle_cosinus - Z * x_angle_sinus;
        Z = y * x_angle_sinus + Z * x_angle_cosinus;
    }

    // ~~

    void RotateAroundY(
        double y_angle_cosinus,
        double y_angle_sinus
        )
    {
        double
            x;

        x = X;
        X = X * y_angle_cosinus + Z * y_angle_sinus;
        Z = Z * y_angle_cosinus - x * y_angle_sinus;
    }

    // ~~

    void RotateAroundZ(
        double z_angle_cosinus,
        double z_angle_sinus
        )
    {
        double
            x;

        x = X;
        X = X * z_angle_cosinus - Y * z_angle_sinus;
        Y = x * z_angle_sinus + Y * z_angle_cosinus;
    }
};

// ~~

struct VECTOR_4
{
    // -- ATTRIBUTES

    double
        X,
        Y,
        Z,
        W;

    // -- OPERATIONS

    void SetNull(
        )
    {
        X = 0.0;
        Y = 0.0;
        Z = 0.0;
        W = 0.0;
    }

    // ~~

    void SetUnit(
        )
    {
        X = 1.0;
        Y = 1.0;
        Z = 1.0;
        W = 1.0;
    }

    // ~~

    void AddVector(
        const VECTOR_4 & vector
        )
    {
        X += vector.X;
        Y += vector.Y;
        Z += vector.Z;
        W += vector.W;
    }

    // ~~

    void MultiplyScalar(
        double scalar
        )
    {
        X *= scalar;
        Y *= scalar;
        Z *= scalar;
        W *= scalar;
    }

    // ~~

    void Translate(
        double x_translation,
        double y_translation,
        double z_translation,
        double w_translation
        )
    {
        X += x_translation;
        Y += y_translation;
        Z += z_translation;
        W += w_translation;
    }

    // ~~

    void Scale(
        double x_scaling,
        double y_scaling,
        double z_scaling,
        double w_scaling
        )
    {
        X *= x_scaling;
        Y *= y_scaling;
        Z *= z_scaling;
        W *= w_scaling;
    }
};

// ~~

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

struct SCAN
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

struct CLOUD
{
    // -- ATTRIBUTES

    TRANSFORM
        Transform;
    vector<SCAN>
        ScanVector;

    // -- OPERATIONS

    void ConvertFile(
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
            point_has_x_field,
            point_has_y_field,
            point_has_z_field,
            point_has_r_field,
            point_has_g_field,
            point_has_b_field,
            point_has_i_field;
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
        string
            output_point_count_text;
        POINT
            point;
        SCAN
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
                            point.ColorVector.X
                                = floor( ( point_r_array[ point_index ] - minimum_r ) * 255 / ( maximum_r - minimum_r ) );
                        }

                        if ( point_has_g_field )
                        {
                            point.ColorVector.Y
                                = floor( ( point_g_array[ point_index ] - minimum_g ) * 255 / ( maximum_g - minimum_g ) );
                        }

                        if ( point_has_b_field )
                        {
                            point.ColorVector.Z
                                = floor( ( point_b_array[ point_index ] - minimum_b ) * 255 / ( maximum_b - minimum_b ) );
                        }

                        if ( point_has_i_field )
                        {
                            point.ColorVector.W
                                = floor( ( point_i_array[ point_index ] - minimum_i ) * 255 / ( maximum_i - minimum_i ) );
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
    CLOUD
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
                cloud.ConvertFile( file_path, argument_array[ 1 ], argument_array[ 2 ], "xyz" );

                argument_count -= 3;
                argument_array += 3;
            }
            else if ( argument_count >= 3
                      && !strcmp( argument_array[ 0 ], "--write-pts-cloud" ) )
            {
                cloud.ConvertFile( file_path, argument_array[ 1 ], argument_array[ 2 ], "pts" );

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
            << "    --write-xyz-cloud <file path>\n"
            << "    --write-pts-cloud <file path> <line format>\n";

        return -1;
    }
}

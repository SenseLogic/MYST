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
#include "E57Foundation.h"

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

    // -- CONSTRUCTORS

    VECTOR_3(
        )
    {
        X = 0.0;
        Y = 0.0;
        Z = 0.0;
    }

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
            x,
            y;

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

    // -- CONSTRUCTORS

    VECTOR_4(
        )
    {
        X = 0.0;
        Y = 0.0;
        Z = 0.0;
        W = 0.0;
    }

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

struct POINT
{
    // -- ATTRIBUTES

    VECTOR_3
        PositionVector;
    VECTOR_4
        ColorVector;

    // -- INQUIRIES

    POINT GetTransformedPoint(
        const VECTOR_3 & position_offset_vector,
        const VECTOR_3 & position_scaling_vector,
        const VECTOR_3 & position_rotation_vector,
        const VECTOR_3 & position_translation_vector,
        const VECTOR_4 & color_offset_vector,
        const VECTOR_4 & color_scaling_vector,
        const VECTOR_4 & color_translation_vector
        ) const
    {
        POINT
            transformed_point;

        transformed_point = *this;

        transformed_point.PositionVector.Translate(
            position_offset_vector.X,
            position_offset_vector.Y,
            position_offset_vector.Z
            );

        transformed_point.PositionVector.Scale(
            position_scaling_vector.X,
            position_scaling_vector.Y,
            position_scaling_vector.Z
            );

        if ( position_rotation_vector.Z != 0.0 )
        {
            transformed_point.PositionVector.RotateAroundZ(
                cos( position_rotation_vector.Z ),
                sin( position_rotation_vector.Z )
                );
        }

        if ( position_rotation_vector.X != 0.0 )
        {
            transformed_point.PositionVector.RotateAroundX(
                cos( position_rotation_vector.X ),
                sin( position_rotation_vector.X )
                );
        }

        if ( position_rotation_vector.Y != 0.0 )
        {
            transformed_point.PositionVector.RotateAroundY(
                cos( position_rotation_vector.Y ),
                sin( position_rotation_vector.Y )
                );
        }

        transformed_point.PositionVector.Translate(
            position_translation_vector.X,
            position_translation_vector.Y,
            position_translation_vector.Z
            );

        transformed_point.ColorVector.Translate(
            color_offset_vector.X,
            color_offset_vector.Y,
            color_offset_vector.Z,
            color_offset_vector.W
            );

        transformed_point.ColorVector.Scale(
            color_scaling_vector.X,
            color_scaling_vector.Y,
            color_scaling_vector.Z,
            color_scaling_vector.W
            );

        transformed_point.ColorVector.Translate(
            color_translation_vector.X,
            color_translation_vector.Y,
            color_translation_vector.Z,
            color_translation_vector.W
            );

        return transformed_point;
    }

    // -- OPERATIONS

    void SetNull(
        )
    {
        PositionVector.SetNull();
        ColorVector.SetNull();
    }

    // ~~

    void SetUnit(
        )
    {
        PositionVector.SetUnit();
        ColorVector.SetUnit();
    }
};

// -- VARIABLES

int
    DecimationCount;
string
    InputFileFormat,
    InputFilePath,
    OutputFileFormat,
    OutputFilePath,
    OutputLineFormat;
VECTOR_3
    PositionOffsetVector,
    PositionRotationVector,
    PositionScalingVector,
    PositionTranslationVector;
VECTOR_4
    ColorOffsetVector,
    ColorScalingVector,
    ColorTranslationVector;

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

void WriteXyzCloudFile(
    ImageFile & image_file,
    CompressedVectorNode & compressed_vector_node
    )
{
    const int
        MaximumPointCount = 4;
    static double
        point_x_array[ MaximumPointCount ],
        point_y_array[ MaximumPointCount ],
        point_z_array[ MaximumPointCount ];
    int
        point_count,
        point_index;
    ofstream
        output_file_stream;
    vector<SourceDestBuffer>
        source_dest_buffer_vector;
    StructureNode
        prototype_structure_node( compressed_vector_node.prototype() );

    cout
        << "Writing file : " << OutputFilePath << "\n";

    output_file_stream.open( OutputFilePath );

    if ( prototype_structure_node.isDefined( "cartesianX" )
         && prototype_structure_node.isDefined( "cartesianY" )
         && prototype_structure_node.isDefined( "cartesianZ" ) )
    {
        source_dest_buffer_vector.push_back( SourceDestBuffer( image_file, "cartesianX", point_x_array, MaximumPointCount, true ) );
        source_dest_buffer_vector.push_back( SourceDestBuffer( image_file, "cartesianY", point_y_array, MaximumPointCount, true ) );
        source_dest_buffer_vector.push_back( SourceDestBuffer( image_file, "cartesianZ", point_z_array, MaximumPointCount, true ) );

        CompressedVectorReader
            compressed_vector_reader = compressed_vector_node.reader( source_dest_buffer_vector );

        while ( ( point_count = compressed_vector_reader.read() ) > 0 )
        {
            for ( point_index = 0;
                  point_index < point_count;
                  ++point_index )
            {
                output_file_stream
                    << point_x_array[ point_index ]
                    << " "
                    << point_y_array[point_index]
                    << " "
                    << point_z_array[point_index]
                    << "\n";
            }
        }

        compressed_vector_reader.close();
    }

    output_file_stream.close();
}

// ~~

void ReadE57CloudFile(
    )
{
    int
        scan_count,
        scan_index;

    cout
        << "Reading file : " << InputFilePath << "\n";

    ImageFile
        image_file( InputFilePath, "r" );
    StructureNode
        root_structure_node = image_file.root();

    if ( root_structure_node.isDefined( "/data3D" ) )
    {
        VectorNode
            vector_node( root_structure_node.get( "/data3D" ) );

        scan_count = vector_node.childCount();

        for ( scan_index = 0; scan_index < scan_count; scan_index++ )
        {
            StructureNode
                structure_node( vector_node.get( scan_index ) );
            CompressedVectorNode
                compressed_vector_node( structure_node.get( "points" ) );

            if ( OutputFileFormat == "xyz" )
            {
                WriteXyzCloudFile( image_file, compressed_vector_node );
            }
        }
    }

    image_file.close();
}

// ~~

int main(
    int argument_count,
    char** argument_array
    )
{
    PositionOffsetVector.SetNull();
    PositionScalingVector.SetUnit();
    PositionRotationVector.SetNull();
    PositionTranslationVector.SetNull();
    ColorOffsetVector.SetNull();
    ColorScalingVector.SetUnit();
    ColorTranslationVector.SetNull();
    DecimationCount = 1;
    InputFilePath = "";
    InputFileFormat = "";
    OutputFilePath = "";
    OutputFileFormat = "";
    OutputLineFormat = "";

    --argument_count;
    ++argument_array;

    while ( argument_count > 0
            && argument_array[ 0 ][ 0 ] == '-'
            && argument_array[ 0 ][ 1 ] == '-' )
    {
        if ( argument_count >= 4
             && !strcmp( argument_array[ 0 ], "--position-offset" ) )
        {
            PositionOffsetVector.X = stod( argument_array[ 1 ] );
            PositionOffsetVector.Y = stod( argument_array[ 2 ] );
            PositionOffsetVector.Z = stod( argument_array[ 3 ] );

            argument_count -= 4;
            argument_array += 4;
        }
        else if ( argument_count >= 4
                  && !strcmp( argument_array[ 0 ], "--position-scaling" ) )
        {
            PositionScalingVector.X = stod( argument_array[ 1 ] );
            PositionScalingVector.Y = stod( argument_array[ 2 ] );
            PositionScalingVector.Z = stod( argument_array[ 3 ] );

            argument_count -= 4;
            argument_array += 4;
        }
        else if ( argument_count >= 4
                  && !strcmp( argument_array[ 0 ], "--position-rotation" ) )
        {
            PositionRotationVector.X = stod( argument_array[ 1 ] );
            PositionRotationVector.Y = stod( argument_array[ 2 ] );
            PositionRotationVector.Z = stod( argument_array[ 3 ] );

            argument_count -= 4;
            argument_array += 4;
        }
        else if ( argument_count >= 4
                  && !strcmp( argument_array[ 0 ], "--position-translation" ) )
        {
            PositionTranslationVector.X = stod( argument_array[ 1 ] );
            PositionTranslationVector.Y = stod( argument_array[ 2 ] );
            PositionTranslationVector.Z = stod( argument_array[ 3 ] );

            argument_count -= 4;
            argument_array += 4;
        }
        else if ( argument_count >= 5
                  && !strcmp( argument_array[ 0 ], "--color-offset" ) )
        {
            ColorOffsetVector.X = stod( argument_array[ 1 ] );
            ColorOffsetVector.Y = stod( argument_array[ 2 ] );
            ColorOffsetVector.Z = stod( argument_array[ 3 ] );
            ColorOffsetVector.W = stod( argument_array[ 4 ] );

            argument_count -= 5;
            argument_array += 5;
        }
        else if ( argument_count >= 2
                  && !strcmp( argument_array[ 0 ], "--color-scaling" ) )
        {
            ColorScalingVector.X = stod( argument_array[ 1 ] );
            ColorScalingVector.Y = stod( argument_array[ 2 ] );
            ColorScalingVector.Z = stod( argument_array[ 3 ] );
            ColorScalingVector.W = stod( argument_array[ 4 ] );

            argument_count -= 5;
            argument_array += 5;
        }
        else if ( argument_count >= 2
                  && !strcmp( argument_array[ 0 ], "--color-translation" ) )
        {
            ColorTranslationVector.X = stod( argument_array[ 1 ] );
            ColorTranslationVector.Y = stod( argument_array[ 2 ] );
            ColorTranslationVector.Z = stod( argument_array[ 3 ] );
            ColorTranslationVector.W = stod( argument_array[ 4 ] );

            argument_count -= 5;
            argument_array += 5;
        }
        else if ( argument_count >= 2
                  && !strcmp( argument_array[ 0 ], "--decimation-count" ) )
        {
            DecimationCount = stoi( argument_array[ 1 ] );

            argument_count -= 2;
            argument_array += 2;
        }
        else if ( argument_count >= 2
                  && !strcmp( argument_array[ 0 ], "--read-e57-cloud" ) )
        {
            InputFilePath = argument_array[ 1 ];
            InputFileFormat = "e57";

            argument_count -= 2;
            argument_array += 2;
        }
        else if ( argument_count >= 2
                  && !strcmp( argument_array[ 0 ], "--write-xyz-cloud" )
                  && OutputFileFormat == "" )
        {
            OutputFilePath = argument_array[ 1 ];
            OutputFileFormat = "xyz";

            argument_count -= 2;
            argument_array += 2;
        }
        else if ( argument_count >= 3
                  && !strcmp( argument_array[ 0 ], "--write-pts-cloud" )
                  && OutputFileFormat == "" )
        {
            OutputFilePath = argument_array[ 1 ];
            OutputFileFormat = "pts";
            OutputLineFormat = argument_array[ 2 ];

            argument_count -= 3;
            argument_array += 3;
        }
        else
        {
            break;
        }
    }

    if ( argument_count == 0
         && InputFileFormat == "e57"
         && OutputFileFormat != "" )
    {
        try
        {
            ReadE57CloudFile();

            return 0;
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
    }

    return -1;
}

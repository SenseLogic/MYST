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

    float
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
        float factor
        )
    {
        X += vector.X * factor;
        Y += vector.Y * factor;
        Z += vector.Z * factor;
    }

    // ~~

    void MultiplyScalar(
        float scalar
        )
    {
        X *= scalar;
        Y *= scalar;
        Z *= scalar;
    }

    // ~~

    void Translate(
        float x_translation,
        float y_translation,
        float z_translation
        )
    {
        X += x_translation;
        Y += y_translation;
        Z += z_translation;
    }

    // ~~

    void Scale(
        float x_scaling,
        float y_scaling,
        float z_scaling
        )
    {
        X *= x_scaling;
        Y *= y_scaling;
        Z *= z_scaling;
    }

    // ~~

    void RotateAroundX(
        float x_angle_cosinus,
        float x_angle_sinus
        )
    {
        float
            y;

        y = Y;
        Y = Y * x_angle_cosinus - Z * x_angle_sinus;
        Z = y * x_angle_sinus + Z * x_angle_cosinus;
    }

    // ~~

    void RotateAroundY(
        float y_angle_cosinus,
        float y_angle_sinus
        )
    {
        float
            x;

        x = X;
        X = X * y_angle_cosinus + Z * y_angle_sinus;
        Z = Z * y_angle_cosinus - x * y_angle_sinus;
    }

    // ~~

    void RotateAroundZ(
        float z_angle_cosinus,
        float z_angle_sinus
        )
    {
        float
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

    float
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
        float scalar
        )
    {
        X *= scalar;
        Y *= scalar;
        Z *= scalar;
        W *= scalar;
    }

    // ~~

    void Translate(
        float x_translation,
        float y_translation,
        float z_translation,
        float w_translation
        )
    {
        X += x_translation;
        Y += y_translation;
        Z += z_translation;
        W += w_translation;
    }

    // ~~

    void Scale(
        float x_scaling,
        float y_scaling,
        float z_scaling,
        float w_scaling
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

    vector<POINT>
        PointVector;

    // -- INQUIRIES

    int64_t const GetPointCount(
        ) const
    {
        return PointVector.size();
    }

    // -- OPERATIONS

    void AddPoint(
        const POINT & point
        )
    {
        PointVector.push_back( point );
    }

    // ~~

    void ReadE57File(
        ImageFile & image_file,
        CompressedVectorNode & compressed_vector_node,
        const TRANSFORM & transform
        )
    {
        const int64_t
            MaximumPointCount = 4;
        static float
            point_x_array[ MaximumPointCount ],
            point_y_array[ MaximumPointCount ],
            point_z_array[ MaximumPointCount ];
        int64_t
            point_count,
            point_index;
        vector<SourceDestBuffer>
            source_dest_buffer_vector;
        StructureNode
            prototype_structure_node( compressed_vector_node.prototype() );
        POINT
            point;

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
                    point.PositionVector.X = point_x_array[ point_index ];
                    point.PositionVector.Y = point_y_array[ point_index ];
                    point.PositionVector.Z = point_z_array[ point_index ];

                    AddPoint( point.GetTransformedPoint( transform ) );
                }
            }

            compressed_vector_reader.close();
        }
    }
};

// ~~

struct CLOUD
{
    // -- ATTRIBUTES

    TRANSFORM
        Transform;
    vector<shared_ptr<SCAN>>
        ScanVector;

    // -- INQUIRIES

    int64_t GetScanCount(
        ) const
    {
        return ScanVector.size();
    }

    // ~~

    void WriteXyzFile(
        const string & file_path
        )
    {
        ofstream
            output_file_stream;

        cout
            << "Writing file : " << file_path << "\n";

        output_file_stream.open( file_path );

        for ( shared_ptr<SCAN> scan : ScanVector )
        {
            for ( POINT point : scan->PointVector )
            {
                output_file_stream
                    << point.PositionVector.X
                    << " "
                    << point.PositionVector.Y
                    << " "
                    << point.PositionVector.Z
                    << "\n";
            }
        }

        output_file_stream.close();
    }

    // ~~

    void WritePtsFile(
        const string & file_path,
        const string & line_format
        )
    {
        int64_t
            point_count;
        ofstream
            output_file_stream;

        cout
            << "Writing file : " << file_path << "\n";

        output_file_stream.open( file_path );

        point_count = 0;

        for ( shared_ptr<SCAN> scan : ScanVector )
        {
            point_count += scan->GetPointCount();
        }

        output_file_stream
            << point_count
            << "\n";

        for ( shared_ptr<SCAN> scan : ScanVector )
        {
            for ( POINT point : scan->PointVector )
            {
                output_file_stream
                    << point.PositionVector.X
                    << " "
                    << point.PositionVector.Y
                    << " "
                    << point.PositionVector.Z
                    << "\n";
            }
        }

        output_file_stream.close();
    }

    // -- OPERATIONS

    void ReadE57File(
        const string & file_path
        )
    {
        int64_t
            scan_count,
            scan_index;
        shared_ptr<SCAN>
            scan;

        cout
            << "Reading file : " << file_path << "\n";

        ImageFile
            image_file( file_path, "r" );
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

                scan = make_shared<SCAN>();
                scan->ReadE57File( image_file, compressed_vector_node, Transform );
                ScanVector.push_back( scan );
            }
        }

        image_file.close();
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
                cloud.ReadE57File( argument_array[ 1 ] );

                argument_count -= 2;
                argument_array += 2;
            }
            else if ( argument_count >= 2
                      && !strcmp( argument_array[ 0 ], "--write-xyz-cloud" ) )
            {
                cloud.WriteXyzFile( argument_array[ 1 ] );

                argument_count -= 2;
                argument_array += 2;
            }
            else if ( argument_count >= 3
                      && !strcmp( argument_array[ 0 ], "--write-pts-cloud" ) )
            {
                cloud.WritePtsFile( argument_array[ 1 ], argument_array[ 2 ] );

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

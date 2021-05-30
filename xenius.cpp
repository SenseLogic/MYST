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

#include <iostream>
#include <fstream>
#include "E57Foundation.h"

using namespace e57;
using namespace std;

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

void WriteFile(
    const string & output_file_path,
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
        << "Writing file : "
        << output_file_path
        << endl;

    output_file_stream.open( output_file_path );

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
                    << endl;
            }
        }

        compressed_vector_reader.close();
    }

    output_file_stream.close();
}

// ~~

void ConvertFile(
    const string & input_file_path,
    const string & output_file_path
    )
{
    int
        scan_count,
        scan_index;

    cout
        << "Reading file : "
        << input_file_path
        << endl;

    ImageFile
        image_file( input_file_path, "r" );
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

            WriteFile( output_file_path, image_file, compressed_vector_node );
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
    --argument_count;
    ++argument_array;

    if ( argument_count == 2
         && HasSuffix( argument_array[ 0 ], ".e57" )
         && ( HasSuffix( argument_array[ 1 ], ".xml" )
              || HasSuffix( argument_array[ 1 ], ".xyz" )
              || HasSuffix( argument_array[ 1 ], ".pts" ) ) )
    {
        try
        {
            ConvertFile( argument_array[ 0 ], argument_array[ 1 ] );

            return 0;
        }
        catch( E57Exception & exception_ )
        {
            cerr
                << "Error :"
                << endl;

            exception_.report( __FILE__, __LINE__, __FUNCTION__ );
        }
        catch ( std::exception & exception_ )
        {
            cerr
                << "Error :"
                << endl
                << exception_.what()
                << endl;
        }
        catch ( ... )
        {
            cerr
                << "Error."
                << endl;
        }
    }
    else
    {
        cerr
            << "Usage:"
            << endl
            << "    xenius file.e57 file.xml"
            << endl
            << "    xenius file.e57 file.xyz"
            << endl
            << "    xenius file.e57 file.pts"
            << endl;
    }

    return -1;
}

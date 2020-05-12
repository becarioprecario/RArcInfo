\name{get.paldata}
\alias{get.paldata}

\title{Function for importing the contents of a PAL file into R}
\description{
This function reads and imports into R the  contents of a polygon definitions file. 
}

\usage{get.paldata(datadir, coverage, filename="pal.adf")}

\arguments{
\item{datadir}{Directory under which all the coverages and a directory called 'info' are.}
\item{coverage}{The name of the coverage we want to work with}
\item{filename}{The name of the file in the coverage directory that
stores the data. By default, it is called 'pal.adf'}
}

\value{
This function returns a list with two elements. The first one is
a data frame with the next fields (columns):

\item{PolygonID}{A number that identifies this polygon.}
\item{MinX}{Minimum value for all the X component of the coordinates.}
\item{MinY}{Minimum value for all the Y component of the coordinates.}
\item{MaxX}{Maximum value for all the X component of the coordinates.}
\item{MaxY}{Maximum value for all the Y component of the coordinates.}
\item{NArcs}{?}

The second element in the list is also a list in which each element is
composed by three arrays with information about the polygons that 
are in the polygon boundary:  'Arc ID', 'From Node' and 'Adjacent Polygon'.
}



\keyword{file}

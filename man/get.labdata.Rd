\name{get.labdata}
\alias{get.labdata}

\title{Funtion for importing  the contents of a LAB file into R}
\description{
This function reads and imports into R the  contents of a polygon labels definition file.
}

\usage{get.labdata(datadir, coverage, filename="lab.adf")}

\arguments{
\item{datadir}{Directory under which iall the coverages and a directory called 'info' are.}
\item{coverage}{The name of the coverage we want to work with}
\item{filename}{The name of the file in the coverage directory that
stores the data. By default, called 'lab.adf'.}
}

\value{
This function returns a data frame with the next fields (by columns):

\item{LabelUserID}{The label of the polygon.}

\item{PolygonID}{The polygon related to the label.}

\item{Coord1X}{The X component of the first coordinate.}

\item{Coord2X}{The X component of the second coordinate.}

\item{Coord3X}{The X component of the third coordinate.}

\item{Coord1Y}{The Y component of the first coordinate.}

\item{Coord2Y}{The Y component of the second coordinate.}

\item{Coord3Y}{The Y component of the third coordinate.}
}

\keyword{file}

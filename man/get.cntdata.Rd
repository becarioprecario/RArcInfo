\name{get.cntdata}
\alias{get.cntdata}

\title{Function for importing the contents of a CNT file into R}
\description{
This function reads and imports into R the  contents of a polygon centroid information file.
}

\usage{get.cntdata(datadir, coverage, filename="cnt.adf")}

\arguments{
\item{datadir}{Directory under which all the coverages and a directory called 'info' are.}
\item{coverage}{The name of the coverage we want to work with.}
\item{filename}{The name of the file in the coverage directory that
stores the data (usually called 'cnt.adf').}
}

\value{
This functions returns a list with two elements. The first one is a data frame
with the next fields (columns):

\item{PolygonID}{The polygon itself.}

\item{CoordX}{An array with the X coordinates of the centroid.}

\item{CoordY}{An array with the Y coordinates of the centroid.}

\item{NLabels}{The number of labels this polygon has.}

The second element in the list is an array with the label identifiers related
to this polygon.
}


\keyword{file}

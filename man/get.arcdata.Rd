\name{get.arcdata}
\alias{get.arcdata}


\title{Function for importing the contents of an ARC file into R}
\description{
This function reads and imports into R the  contents of an arcs definition file.
}


\usage{get.arcdata(datadir, coverage, filename="arc.adf")}

\arguments{
\item{datadir}{Directory under which all the coverages and a directory called 'info' are.}
\item{coverage}{The name of the coverage we want to work with.}
\item{filename}{The name of the file in the coverage directory that
stores the data. By default, it is called 'arc.dat'.}
}

\value{
This function returns a list with two elements. The first one is a data
frame containing the next fields (by columns):

\item{ArcID}{A number that identifies this arc.}

\item{ArcUserID}{Identifier defined by the user.}

\item{FromNode}{The node where the arc begins.}

\item{ToNode}{The node where the arc finishes.}

\item{LeftPoly}{The number of the polygon that is to the left of the arc.}

\item{RightPoly}{The number of the polygon that is to the right of the arc.}

\item{NVertices}{The number of vertices the arc has.}

The second element is a list that stores the vertices of the arc. So, each element
in this list is also a list of two arrays: the first for the X coordinates
and the secod for the Y coordinates.
}



\keyword{file}

\name{get.nb}
\alias{get.nb}


\title{Function for calculating neighbouting polygons.}
\description{
This function allows the user to calculate, for every given polygon (in the
array 'index'), the list of its neighbouring polygons.  

}


\usage{get.nb(arc,pal, index=NULL)}

\arguments{
\item{arc}{The list of arc definitions, as returned by 'get.arcdata'.}
\item{pal}{The list of polygon definitions, as returned by 'get.paldata'.}
\item{index}{An array with the polygons we want to use to calculate their
neighbours. It must be an array. If 'index' is not set, then all the polygons
are used.}
}

\value{
A list in which the first element is a vector of the neighbouring polygons
of the fist element in 'index', and so on.
}


\keyword{file}

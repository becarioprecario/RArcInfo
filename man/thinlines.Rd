\name{thinlines}
\alias{thinlines}
\alias{thinl}

\title{Function for deleting points in an arc definition object}
\description{
Usually the plotting device resolution allows us to plot only a few
points per arc with no difference.
}

\usage{thinlines(arc, tol)}

\arguments{
\item{arc}{The original arc definition object, as retuend by get.arcdata.}
\item{tol}{The theshold we used to define which polygons are 'too close'.}
}


\value{
A new arc definition object, usually with less points per arc than the original.
Notice that no modification is made to the topology but the number of
vertices is updated in the table related to the arc definition.
}

\keyword{file}

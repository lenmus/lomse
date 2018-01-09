/**

@page page-file-formats File formats supoported by Lomse

@tableofcontents

@section page-file-formats-overview Supported file formats

Lomse supports the following file formats:

- <b>LMS: LenMus Score (.lms extension):</b>
    Plain txt file containing only a music score writen in LDP language. LDP is the acronym for Spanish <i>Lenguaje De Partituras</i> (<i>Language for music scores</i>). It was the initial language which started LenMus project.

- <b>LMD: LenMus Document (.lmd extension):</b>
    Plain XML file containing a full document with headers, paragraphs, images, tables lists, scores, etc. It is similar to an HTML or DocBook file with scores in LDP language (tag \<ldpmusic\>) or MusicXML (tag \<mxlmusic\>)
    
- <b>MusicXML:  (.xml extension):</b>
    An XML-based file format for representing Western musical notation, widely used by all music applications as interchange format.



@subsection ldp-format LDP format

The LenMus LDP notation language is a general purpose formal language for representing music scores in plain-text in a human-readable way. The name is an acronym from the Spanish sentence <i>"Lenguaje de Descripción de Partituras"</i>, that is <i>"Language for music scores"</i>. 

It was the initial language which started LenMus project in 2002. All LDP files (.lms extension) are Unicode text files, encoded using the UTF-8 encoding scheme.

Let's see how a very simple score is encoded. The score is just one measure, in G clef, key signature will be C major (no accidentals) and time signature will be 4 by 4. The measure will have a whole C note. Here is how it would look:

@image html ldp-hello-world-score.jpg "Image: Rendering of the following LDP score."

In LDP this score is described as follows:

@code
(score
    (vers 2.0)
    (instrument
        (musicData
            (clef G)
            (key C)
            (time 4 4)
            (n c4 w)
            (barline)
        )
    )
)
@endcode

For more information see the LDP manual at https://lenmus.github.io/ldp/ .


@subsection lmd-format LMD format

As LenMus project evolved, LDP was insuficient as it was necessary to describe not only music scores but full documents containing scores, headers, paragraphs, lists, tables, images, etc. This was a requirement for writing music eBooks. In those days (2005), in was not easy to mix music scores in HTML documents and the solution was to create an XML format for describing whole documents. The format is similar to that of a HTML or DocBook file. For supporting scores in different formats a tag for each format is created. For instance, tag \<ldpmusic\> is used to include LDP scores.

@image html lmd-sample-document.png "Image: Rendering of the following LMD document."

This is an example of an LMD file:

@code
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE lenmusdoc PUBLIC
    "-//LenMus//DTD LenMus Document 0.0//EN" 
    "https://raw.githubusercontent.com/lenmus/lomse/master/dtd/lenmusdoc-0.0.dtd" >
<lenmusdoc vers="0.0" language="en">
   <styles>
      <defineStyle>
         <name>eBook_para</name>
         <margin-bottom>600</margin-bottom>
      </defineStyle>
      <defineStyle>
         <name>bold-red</name>
         <font-weight>bold</font-weight>
         <color>#ff0000</color>
      </defineStyle>
   </styles>

   <content> 
      <section level="1">A simple LMD document</section> 

      <para style="eBook_para">
        Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor
        incididunt ut labore et dolore magna aliqua. 
        <txt style="bold-red">Ut enim ad minim veniam</txt>, quis nostrud
        exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.
      </para> 

      <ldpmusic>
           (score (vers 2.0)
              (systemLayout first 
                 (systemMargins 0 0 2000 540))
              (instrument 
                 (staves 1)
                 (musicData 

                    // measure 1
                    (clef G)
                    (key A)
                    (time 6 8)
                    (n f5 q. l)(n f5 s g+ t7/6)
                    (n e5 s)(n +d5 s)(n e5 s)(n +e5 s)(n g5 s.)(n f5 s g- t-)
                    (barline)

                    // measure 2
                    (chord (n a4 q.)(n c5 q.)(n e5 q.))
                    (n a4 q)(n c5 e)
                    (barline)

                    // measure 3
                    (n a5 q (fermata above))
                    (r q)
                    (barline end)
                 )
              )
           )
      </ldpmusic>

      <para>
        Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor
        incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud
        exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.
      </para> 

   </content>
</lenmusdoc>
@endcode

For more information see the DTD for LMD at https://github.com/lenmus/lomse/dtd/lenmusdoc-0.0.dtd.


@subsection mxl-format MusicXML format

MusicXML is a widely use format for music scores. It was designed for the interchange of scores between different music notation. 
The following example is a score consisting of a single whole note middle C, with 4/4 time signature, in the key of C major, on the G clef:

@image html ldp-hello-world-score.jpg "Image: Rendering of the following MusicXML score."

In MusicXML this score is described as follows:

@code
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!DOCTYPE score-partwise PUBLIC
    "-//Recordare//DTD MusicXML 3.0 Partwise//EN"
    "http://www.musicxml.org/dtds/partwise.dtd">
<score-partwise version="3.0">
  <part-list>
    <score-part id="P1">
      <part-name>Music</part-name>
    </score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key>
          <fifths>0</fifths>
        </key>
        <time>
          <beats>4</beats>
          <beat-type>4</beat-type>
        </time>
        <clef>
          <sign>G</sign>
          <line>2</line>
        </clef>
      </attributes>
      <note>
        <pitch>
          <step>C</step>
          <octave>4</octave>
        </pitch>
        <duration>4</duration>
        <type>whole</type>
      </note>
    </measure>
  </part>
</score-partwise>
@endcode

For more information visit http://www.musicxml.com/


*/

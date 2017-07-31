// MessageSender.java (partial implementation)

/**
 * This class implements the sender side of the data link layer.
 * <P>
 * The source code supplied here contains only a partial implementation. 
 * Your completed version must be submitted for assessment.
 * <P>
 * You only need to finish the implementation of the sendMessage
 * method to complete this class.  No other parts of this file need to
 * be changed.  Do NOT alter the constructor or interface of any public
 * method.  Do NOT put this class inside a package.  You may add new
 * private methods, if you wish, but do NOT create any new classes. 
 * Only this file will be processed when your work is marked.
 */

public class MessageSender
{
    // Fields ----------------------------------------------------------

    private FrameSender physicalLayer;   // physical layer object
    private boolean quiet;               // true=quiet mode (suppress
                                         // prompts and status info)

    // You may add additional fields but this shouldn't be necessary

    // Constructor -----------------------------------------------------

    /**
     * MessageSender constructor (DO NOT ALTER ANY PART OF THIS)
     * Create and initialize new MessageSender.
     * @param physicalLayer physical layer object with frame sender service
     * (this will already have been created and initialized by TestSender)
     * @param quiet true for quiet mode which suppresses prompts and status info
     */

    public MessageSender(FrameSender physicalLayer, boolean quiet)
    {
        // Initialize fields and report status

        this.physicalLayer = physicalLayer;
        this.quiet = quiet;
        if (!quiet) {
            System.out.println("Data link layer ready");
        }
    }

    // Methods ---------------------------------------------------------

    /**
     * Send a message (THIS IS THE ONLY METHOD YOU NEED TO MODIFY)
     * @param message the message to be sent.  The message can be any
     * length and may be empty but the string reference should not
     * be null.
     * @throws ProtocolException immediately without attempting to
     * send any further frames if (and only if) the physical layer
     * throws an exception or the given message can't be sent
     * without breaking the rules of the protocol (e.g. if the MTU
     * is too small)
     */

    public void sendMessage(String message) throws ProtocolException
    {
        // Announce action
        // (Not required by protocol but helps when debugging)
        // System.out.println("Sending message => " + message);
        String frame = "";
        boolean last = false;
        
        if (!quiet) {
            System.out.println("Sending message => " + message);
        }
        
        while (!last) {
            // error Carries validation feedback
            String errorMessage = validateMessage(message);
            if(errorMessage != null)
                throw new ProtocolException(
                    "Message: incorrect " + errorMessage + "please re-enter."
                );
            // messageLength The length of message being sent
            int messageLength = message.length();
            // mtu the MTU size, used for building the segments and frames
            int mtu = physicalLayer.getMTU();
            // The maximum length of the segment: (frame - fixed chars)=segment. 
            int maxSegmentLength = mtu - 10; 
            String mtuErrorMessage = validateMTU(mtu);
            if (mtuErrorMessage != null)
                throw new ProtocolException(
                    "MTU: incorrect " + mtuErrorMessage + "please exit and reset MTU."
                );
            if ( (messageLength >= 0) && (messageLength <= maxSegmentLength) ) {
                // format the integer with 2 digits
                String segLength = String.format( "%02d", messageLength ); 
                frame = "E" + "-" + segLength + "-" + message + "-"; 
                String checkSum = generateCheckSum(frame);
                frame = "<" + frame + checkSum + ">";
                physicalLayer.sendFrame(frame);
                last = true; 
            } else if (messageLength > maxSegmentLength) {
                String segLength = String.format( "%02d", maxSegmentLength); 
                frame = "D" + "-" + segLength + "-"; 
                String msgSegment = message.substring(0, maxSegmentLength);
                frame = frame + msgSegment + "-";
                String checkSum = generateCheckSum (frame);
                frame = "<" + frame + checkSum + ">";
                physicalLayer.sendFrame(frame);
                message = message.substring(maxSegmentLength, message.length());
                last = false; 
            }
            // frame = "<" + frameType + "-" + segLength + "-" + message + "-" 
            //     + checkSum + ">";
        }
    } // end of method sendMessage
        
    /**
     * Validates the entered message to ensure of correctness. 
     * @param message to be validated
     * @return error that the message has
     */
    private String validateMessage(String message)
    {
        String errorMessage = "";
        if(message != null) {
            // message is okay ... 
        } else {
            errorMessage += "message is empty ";
        }
        if (errorMessage.length() == 0)
            return null;
        else 
            return errorMessage;
        // validation for frame, debugging
        // matches with the regular expression of the format
        //if (!message.matches("<[E|D]-\\d{2}-.{1,99}-\\d{2}>")) 
        //  errorMessage += "frame structure, ";
    }
    
    /**
     * Validates the MTU value to ensure that it is of the correct length,
     * i.e. not too small (larger than 10). 
     * @param mtu to be validated
     * @return error that the mtu has
     */
    private String validateMTU(int mtu)
    {
        String errorMessage = "";
        if (mtu != 0) {
            if (mtu <= 10) {
                errorMessage += "MTU is too small ";
            }
            if (errorMessage.length() == 0)
                return null;
            return errorMessage;
        } else { 
            return ("MTU has not been set "); 
        }
    }

    /**
     * Takes a frame and calculates the check sum value by the arithmetic 
     * sum of the characters inside the frame.
     * @param frame the frame which is having it's check sum calculated
     * @return the checkSum value of the frame
     */
    private String generateCheckSum(String frame)
    {
        //System.out.println("frame: " + frame);
        int checkSum = 0;
        for(int i = 0; i < frame.length(); i++) {
            checkSum = checkSum + frame.charAt(i);      
        }        
        String csString = Integer.toString(checkSum);
        String csString2 = csString.substring(csString.length()-2, csString.length());
        return csString2;
    }
}
 // end of class MessageSender
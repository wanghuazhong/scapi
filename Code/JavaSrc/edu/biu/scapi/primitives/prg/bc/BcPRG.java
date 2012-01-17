package edu.biu.scapi.primitives.prg.bc;

import java.security.spec.AlgorithmParameterSpec;

import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;

import org.bouncycastle.crypto.CipherParameters;
import org.bouncycastle.crypto.StreamCipher;

import edu.biu.scapi.exceptions.UnInitializedException;
import edu.biu.scapi.primitives.prg.PseudorandomGeneratorAbs;
import edu.biu.scapi.tools.Translation.BCParametersTranslator;

/**
 * A general adapter class of PRG for bouncy castle. <p>
 * This class implements the PRG functionality by passing requests to the adaptee interface StreamCigher.
 * A concrete prg such as RC4 represented in the class BcRC4only passes the RC4Engine object in the constructor.
 * 
 * @author Cryptography and Computer Security Research Group Department of Computer Science Bar-Ilan University (Meital Levy)
 *
 */
public abstract class BcPRG extends PseudorandomGeneratorAbs {
	
	private StreamCipher bcStreamCipher;	//the underlying stream cipher of bc
	private CipherParameters bcParams;		//the parameters for the underlying StreamCipher
		

	/** 
	 * Sets the StreamCipher of bc to adapt to.
	 * @param bcStreamCipher - the concrete StreamCipher of bc
	 */
	public BcPRG(StreamCipher bcStreamCipher) {
		this.bcStreamCipher = bcStreamCipher;
	}
	
	public void init(SecretKey secretKey) {
		
		//gets the BC keyParameter relevant to the secretKey
		bcParams = BCParametersTranslator.getInstance().translateParameter(secretKey);
		
		//initializes the underlying stream cipher. Note that the first argument is irrelevant and thus does not matter is true or false.
		bcStreamCipher.init(false, bcParams);
		
		//sets the parameters
		super.init(secretKey);
	}

	public void init(SecretKey secretKey, AlgorithmParameterSpec params) {
		
		//sends the parameters converted to bc.
		bcParams = BCParametersTranslator.getInstance().translateParameter(secretKey, params);
		
		//initializes the underlying stream cipher. Note that the first argument is irrelevant and thus does not matter is true or false.
		bcStreamCipher.init(false, bcParams);
		
		//sets the parameters
		super.init(secretKey, params);
	}
	
	/** 
	 * Returns the name of the algorithm through the underlying StreamCipher
	 * @return - the algorithm name
	 */
	public String getAlgorithmName() {
		
		return bcStreamCipher.getAlgorithmName();
	}

	/** 
	 * Streams the bytes using the underlying stream cipher.
	 * @param outBytes - output bytes. The result of streaming the bytes.
	 * @param outOffset - output offset
	 * @param outlen - the required output length
	 * @throws UnInitializedException if this object is not initialized
	 */
	public void getPRGBytes(byte[] outBytes, int outOffset,	int outLen) throws UnInitializedException {
		if (!isInitialized()){
			throw new UnInitializedException();
		}
		//checks that the offset and the length are correct
		if ((outOffset > outBytes.length) || ((outOffset + outLen) > outBytes.length)){
			throw new ArrayIndexOutOfBoundsException("wrong offset for the given output buffer");
		}
		
		/*
		 * BC generates bytes and does XOR between them to the given byte array. 
		 * In order to get the bytes without XOR we send a zeroes array to be XOR-ed with the generated bytes.
		 * Because XOR with zeroes returns the input to the XOR - we will get the generated bytes.
		 */
		
		//in array filled with zeroes
		byte[] inBytes = new byte[outLen];
		
		//out array filled with pseudorandom bytes (that were xored with zeroes in the in array)
		bcStreamCipher.processBytes(inBytes, 0, outLen, outBytes, outOffset);
	}


}